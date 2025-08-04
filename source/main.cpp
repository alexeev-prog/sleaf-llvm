#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "_default.hpp"
#include "input_parser.hpp"
#include "logger.hpp"

namespace fs = std::filesystem;

namespace {
    /**
     * @brief Check if util is available (cross-platform)
     */
    auto is_util_available(const std::string& util) -> bool {
#ifdef _WIN32
        std::string cmd = "where " + util + " >nul 2>nul";
#else
        std::string cmd = "command -v " + util + " >/dev/null 2>&1";
#endif
        return std::system(cmd.c_str()) == 0;
    }

    /**
     * @brief Safe command execution
     */
    auto execute_command(const std::string& cmd, bool quiet = true) -> int {
        if (quiet) {
#ifdef _WIN32
            return std::system((cmd + " >nul 2>nul").c_str());
#else
            return std::system((cmd + " >/dev/null 2>&1").c_str());
#endif
        } else {
            return std::system(cmd.c_str());
        }
    }

    /**
     * @brief Generate safe quoted path
     */
    auto safe_path(const std::string& path) -> std::string {
        if (path.empty()) {
            return "\"\"";
        }
        if (path.find(' ') != std::string::npos) {
            return "\"" + path + "\"";
        }
        return path;
    }

    /**
     * @brief Compile generated IR to binary
     */
    auto compile_ir(const std::string& output_base) -> bool {
        const std::string ll_file = output_base + ".ll";
        const std::string opt_ll_file = output_base + "-opt.ll";
        const std::string bin_file = output_base;

        if (!fs::exists(ll_file)) {
            LOG_ERROR("IR code not found");
            return false;
        }

        std::string opt_cmd = "opt " + safe_path(ll_file) + " -O3 -S -o " + safe_path(opt_ll_file);

        LOG_INFO("Optimizing code...");

        if (execute_command(opt_cmd) != 0) {
            LOG_ERROR("Code optimization failed");
            std::cout << "Command: " << opt_cmd << "\n";
            execute_command(opt_cmd, false);
            return false;
        }

        if (!fs::exists(opt_ll_file) || fs::file_size(opt_ll_file) == 0) {
            LOG_ERROR("Optimized IR code not created");
            return false;
        }

        std::string clang_cmd = "clang++ -O3 " + safe_path(opt_ll_file) + " -o " + safe_path(bin_file);

        LOG_INFO("Compiling optimized code...");

        if (execute_command(clang_cmd) != 0) {
            LOG_ERROR("Binary compilation failed");
            std::cout << "Command: " << clang_cmd << "\n";
            execute_command(clang_cmd, false);
            return false;
        }

        if (!fs::exists(bin_file) || fs::file_size(bin_file) == 0) {
            LOG_ERROR("Binary file \"%s\" not created", bin_file.c_str());
            return false;
        }

        return true;
    }

    /**
     * @brief Safe cleanup of temporary files
     */
    void cleanup_temp_files(const std::string& output_base) {
        auto safe_remove = [](const std::string& path)
        {
            try {
                if (fs::exists(path)) {
                    fs::remove(path);
                    LOG_DEBUG("Removed temp file: %s", path.c_str());
                }
            } catch (...) {
                LOG_WARN("Could not remove file \"%s\"", path.c_str());
            }
        };

        safe_remove(output_base + ".ll");
        safe_remove(output_base + "-opt.ll");
    }

    /**
     * @brief Check if all required utils are available
     */
    auto check_utils_available() -> bool {
        const std::vector<std::string> REQUIRED_PROGS = {"opt", "clang++"};

        for (const auto& util : REQUIRED_PROGS) {
            if (!is_util_available(util)) {
                LOG_ERROR("Required utility \"%s\" not found. Please install it.", util.c_str());
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Check if output name is valid
     */
    auto is_valid_output_name(const std::string& name) -> bool {
        if (name.empty()) {
            return false;
        }

        const std::string FORBIDDEN_CHARS = "/\\:*?\"<>|";
        return std::none_of(
            name.begin(), name.end(), [&](char c) { return FORBIDDEN_CHARS.find(c) != std::string::npos; });
    }
}    // namespace

/**
 * @brief Entry point
 */
auto main(int argc, char** argv) -> int {
    bool compile_raw_object_file = false;

    // Initialize parser with program info
    InputParser parser(fs::path(argv[0]).filename().string(),
                       "SLeaf-LLVM - Compiler for the SLeaf programming language");

    // Register command line options
    parser.add_option({"-v", "--version", "Get version", false, ""});
    parser.add_option({"-h", "--help", "Print this help message", false, ""});

    // Parse command line
    if (!parser.parse(argc, argv)) {
        for (const auto& error : parser.get_errors()) {
            LOG_ERROR("%s", error.c_str());
        }
        std::cerr << parser.generate_help() << "\n";
        return 1;
    }

    if (parser.has_option("-v")) {
        LOG_INFO("Version: %s", VERSION);
        return 0;
    }

    // Handle help option
    if (parser.has_option("-h") || parser.has_option("--help")) {
        std::cout << parser.generate_help() << "\n";
        return 0;
    }

    // Check required utilities
    if (!check_utils_available()) {
        return 1;
    }

    return 0;
}
