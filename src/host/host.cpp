/*
Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
*/

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_graph.h>
#include <xrt/xrt_kernel.h>
#include "../config.h"

template <typename T>
std::vector<T> load_data(const std::string& filename, std::size_t count) {
    std::ifstream file(filename);
    if (!file)
        throw std::runtime_error("Cannot open " + filename);
    std::vector<T> data(count);
    for (auto& element : data) {
        std::int64_t value;
        if (!(file >> value) || value < std::numeric_limits<T>::min() ||
            value > std::numeric_limits<T>::max())
            throw std::runtime_error("Missing or invalid data in " + filename);
        element = static_cast<T>(value);
    }
    file >> std::ws;
    if (!file.eof())
        throw std::runtime_error("Unexpected extra data in " + filename);
    return data;
}

template <typename T>
std::size_t compare(const T* output, const std::vector<T>& reference,
                    const char* name) {
    std::size_t errors = 0;
    for (std::size_t i = 0; i < reference.size(); ++i) {
        if (output[i] != reference[i]) {
            if (errors < 10)
                std::cerr << name << '[' << i << "]: expected " << +reference[i]
                          << ", got " << +output[i] << '\n';
            ++errors;
        }
    }
    std::cout << name << ": " << errors << " mismatches\n";
    return errors;
}

void wait_for_kernel(xrt::run& run, const char* name) {
    const auto state = run.wait();
    if (state != ERT_CMD_STATE_COMPLETED)
        throw std::runtime_error(std::string(name) + " failed, state=" +
                                 std::to_string(static_cast<int>(state)));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <xclbin>\n";
        return EXIT_FAILURE;
    }
    try {
        constexpr std::size_t count_a = std::size_t(NITERATIONS) * A_ROWS * A_COLS;
        constexpr std::size_t count_b = std::size_t(NITERATIONS) * A_COLS * B_COLS;
        constexpr std::size_t count_c = std::size_t(NITERATIONS) * A_ROWS * B_COLS;

        // Validate every file before starting any hardware operation.
        const auto a = load_data<std::int8_t>("data/inputA_128.txt", count_a);
        const auto b = load_data<std::int8_t>("data/inputB_128.txt", count_b);
        const auto ref32 = load_data<std::int32_t>("data/outputC_ref_128_32b.txt", count_c);
        const auto ref16 = load_data<std::int16_t>("data/outputC_ref_128_16b.txt", count_c);

        auto device = xrt::device(0);
        const auto uuid = device.load_xclbin(argv[1]);
        auto kernel = xrt::kernel(device, uuid, "top:{top_1}");

        auto input_a = xrt::bo(device, a.size(), kernel.group_id(0));
        auto input_b = xrt::bo(device, b.size(), kernel.group_id(1));
        auto c32 = xrt::bo(device, count_c * sizeof(std::int32_t), kernel.group_id(2));
        auto c16 = xrt::bo(device, count_c * sizeof(std::int16_t), kernel.group_id(3));

        std::copy(a.begin(), a.end(), input_a.map<std::int8_t*>());
        std::copy(b.begin(), b.end(), input_b.map<std::int8_t*>());
        input_a.sync(XCL_BO_SYNC_BO_TO_DEVICE);
        input_b.sync(XCL_BO_SYNC_BO_TO_DEVICE);

        auto graph = xrt::graph(device, uuid, "GMatMult");
        // DATAFLOW moves inputs and outputs concurrently while the AIE graph runs.
        auto run = kernel(input_a, input_b, c32, c16,
                          nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        std::cout << "Running " << NITERATIONS << " iterations: "
                  << A_ROWS << 'x' << A_COLS << " * " << A_COLS << 'x' << B_COLS << '\n';
        graph.run(NITERATIONS);
        graph.wait();
        graph.end(0);
        wait_for_kernel(run, "top");
        c32.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        c16.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        const auto errors32 = compare(c32.map<std::int32_t*>(), ref32, "int32");
        const auto errors16 = compare(c16.map<std::int16_t*>(), ref16, "int16 (shift=6)");
        const bool failed = errors32 + errors16 != 0;
        std::cout << (failed ? "TEST FAILED\n" : "TEST PASSED\n");
        return failed ? EXIT_FAILURE : EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "TEST FAILED: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
