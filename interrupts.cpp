/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <interrupts.hpp>
#include <vector>
#include "interrupts.hpp"

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: ./interrupts <trace.txt> <vector_table.txt> <device_table.txt>\n";
        return 1;
    }

    auto [vectors, delays] = parse_args(argc, argv); // load vector and device tables

    std::ifstream input_file(argv[1]);
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open trace file." << std::endl;
        return 1;
    }

    long long current_time = 0;
    std::string trace;
    std::string execution;

    int context_time = 20;    
    int isr_time = 40;        // ISR execution time, values will change from 40–200
    int switch_time = 1;      // switch form kernel/user mode 
    int iret_time = 1;        // IRET instruction time

    while (std::getline(input_file, trace)) { // this reads each trace line
        auto [activity, value] = parse_trace(trace);

        if (activity == "CPU") {
            execution += std::to_string(current_time) + ", " + std::to_string(value) + ", CPU burst\n";
            current_time += value;
        }

        else if (activity == "SYSCALL" || activity == "END_IO") {
            int intr_num = value;

            // Interrupt entry sequence
            auto [log_str, updated_time] = intr_boilerplate(current_time, intr_num, context_time, vectors);
            execution += log_str;
            current_time = updated_time;

            // ISR execution
            execution += std::to_string(current_time) + ", " + std::to_string(isr_time) + ", " + activity + ": run the ISR (device driver)\n";
            current_time += isr_time;

            // Return from interrupt only on END_IO
            if (activity == "END_IO") {
                execution += std::to_string(current_time) + ", " + std::to_string(iret_time) + ", IRET\n";
                current_time += iret_time;

                execution += std::to_string(current_time) + ", " + std::to_string(context_time) + ", context restored\n";
                current_time += context_time;

                execution += std::to_string(current_time) + ", " + std::to_string(switch_time) + ", switch to user mode\n";
                current_time += switch_time;
            }
        }
    }

    input_file.close();
    write_output(execution);
    return 0;
}
