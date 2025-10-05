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

long long current_time = 0;
const int context_time = 10;
std::map<int, std::string> vector_table;
std::map<int, int> device_table;
std::ifstream trace_file("trace.txt");
std::ofstream execution_file("execution.txt");

int main(int argc, char** argv) {
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;
    std::string execution;

    // input file opened.
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open trace file." << std::endl;
        return 1;
    }

    // parsing each line.
    while (std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        if (activity == "CPU") {
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU burst\n";
            current_time += duration_intr;
        } 
        else if (activity == "SYSCALL") {
            auto [log_str, updated_time] = intr_boilerplate(current_time, duration_intr, context_time, vectors);
            execution += log_str;
            current_time = updated_time;

            execution += std::to_string(current_time) + ", 40, SYSCALL: run the ISR (device driver)\n";
            current_time += 40;

            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
        }
        else if (activity == "END_IO") {

            auto [log_str, updated_time] = intr_boilerplate(current_time, duration_intr, context_time, vectors);
            execution += log_str;
            current_time = updated_time;
            
            
            execution += std::to_string(current_time) + ", 40, ENDIO: run the ISR (device driver)\n";
            current_time += 40;
            
        
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
            
            execution += std::to_string(current_time) + ", 10, context restored\n";
            current_time += 10;
            
            execution += std::to_string(current_time) + ", 1, switch to user mode\n";
            current_time += 1;
        }
    }

    input_file.close();
    write_output(execution);

    return 0;
}
