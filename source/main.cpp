#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/cxx11/none_of.hpp>

#include "_default.hpp"
#include "absl/strings/match.h"
#include "input_parser.hpp"
#include "logger.hpp"
#include "ast/ast.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

using namespace sleaf;

namespace fs = std::filesystem;

namespace {
    auto is_util_available(const std::string& util) -> bool {
#ifdef _WIN32
        std::string cmd = "where " + util + " >nul 2>nul";
#else
        std::string cmd = "command -v " + util + " >/dev/null 2>&1";
#endif
        return std::system(cmd.c_str()) == 0;
    }

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

    auto safe_path(const std::string& path) -> std::string {
        if (path.empty()) return "\"\"";
        if (absl::StrContains(path, ' ')) return "\"" + path + "\"";
        return path;
    }

    auto compile_ir(const std::string& output_base) -> bool {
        const std::string LL_FILE = output_base + ".ll";
        const std::string OPT_LL_FILE = output_base + "-opt.ll";
        const std::string& bin_file = output_base;

        if (!fs::exists(LL_FILE)) {
            LOG_ERROR("IR code not found");
            return false;
        }

        std::string opt_cmd = "opt " + safe_path(LL_FILE) + " -O3 -S -o " + safe_path(OPT_LL_FILE);
        LOG_INFO("Optimizing code...");

        if (execute_command(opt_cmd) != 0) {
            LOG_ERROR("Code optimization failed");
            std::cout << "Command: " << opt_cmd << "\n";
            execute_command(opt_cmd, false);
            return false;
        }

        if (!fs::exists(OPT_LL_FILE) || fs::file_size(OPT_LL_FILE) == 0) {
            LOG_ERROR("Optimized IR code not created");
            return false;
        }

        std::string clang_cmd = "clang++ -O3 " + safe_path(OPT_LL_FILE) + " -o " + safe_path(bin_file);
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

    void cleanup_temp_files(const std::string& output_base) {
        auto safe_remove = [](const std::string& path) {
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

    auto is_valid_output_name(const std::string& name) -> bool {
        if (name.empty()) return false;
        const std::string FORBIDDEN_CHARS = "/\\:*?\"<>|";
        return boost::algorithm::none_of(name, [&](char c) {
            return absl::StrContains(FORBIDDEN_CHARS, c);
        });
    }

    auto format_token(const Token& token) -> std::string {
        std::ostringstream stringstream;
        stringstream << "[" << std::setw(3) << token.line << ":" << std::setw(3) << token.column << "] "
                     << std::setw(20) << std::left << token.type_name() << " '" << token.lexeme << "'";
        return stringstream.str();
    }

    auto read_source(const std::string& filename) -> std::string {
        if (filename.empty()) {
            std::cout << "Enter SLEAF code (Ctrl+D to finish):\n";
            return std::string(std::istreambuf_iterator<char>(std::cin),
                               std::istreambuf_iterator<char>());
        }

        std::ifstream file(filename);
        if (!file.is_open()) {
            LOG_CRITICAL("Could not open file: %s", filename.c_str());
            return "";
        }
        return std::string(std::istreambuf_iterator<char>(file),
                           std::istreambuf_iterator<char>());
    }

    auto run_lexer(const std::string& source) -> int {
        if (source.empty()) {
            LOG_ERROR("No source code provided");
            return 1;
        }

        Lexer lexer(source);
        std::cout << "\nToken stream:\n----------------------------------------\n";

        int token_count = 0;
        while (true) {
            Token token = lexer.scan_token();
            std::cout << format_token(token) << "\n";

            if (token.type == TokenType::END_OF_FILE) break;
            if (token.type == TokenType::ERROR) {
                std::cerr << "Lexical error: " << token.lexeme << "\n";
            }

            if (++token_count > 500) {
                std::cerr << "Token limit exceeded\n";
                break;
            }
        }
        return 0;
    }

    class ASTPrinter : public ASTVisitor {
        int indent = 0;

        void print_indent() {
            for (int i = 0; i < indent; ++i) std::cout << "  ";
        }

    public:
        void visit(BlockStmt& node) override {
            print_indent();
            std::cout << "Block:\n";
            indent++;
            for (auto& stmt : node.statements) stmt->accept(*this);
            indent--;
        }

        void visit(FunctionDecl& node) override {
            print_indent();
            std::cout << "Function: " << node.name << "\n";
            indent++;
            node.body->accept(*this);
            indent--;
        }

        void visit(IfStmt& node) override {
            print_indent();
            std::cout << "If:\n";
            indent++;
            node.condition->accept(*this);
            node.then_branch->accept(*this);
            if (node.else_branch) node.else_branch->accept(*this);
            indent--;
        }

        void visit(BinaryExpr& node) override {
            print_indent();
            std::cout << "Binary: " << static_cast<int>(node.op) << "\n";
            indent++;
            node.left->accept(*this);
            node.right->accept(*this);
            indent--;
        }

        void visit(Literal& node) override {
            print_indent();
            std::cout << "Literal: " << node.value << "\n";
        }

        void visit(Identifier& node) override {
            print_indent();
            std::cout << "Identifier: " << node.name << "\n";
        }

        void visit(ExpressionStmt& node) override {
            print_indent();
            std::cout << "ExpressionStmt:\n";
            indent++;
            node.expr->accept(*this);
            indent--;
        }

        void visit(WhileStmt&) override {}
        void visit(ForStmt&) override {}
        void visit(ReturnStmt&) override {}
        void visit(VarDecl&) override {}
        void visit(Parameter&) override {}
        void visit(AssignExpr&) override {}
        void visit(UnaryExpr&) override {}
        void visit(CallExpr&) override {}
        void visit(GroupingExpr&) override {}
    };

    auto run_parser(const std::string& source) -> int {
        if (source.empty()) {
            LOG_ERROR("No source code provided");
            return 1;
        }

        Lexer lexer(source);
        Parser parser(lexer);
        auto statements = parser.parse();

        if (parser.had_error()) {
            LOG_ERROR("Parsing failed with %d errors", parser.had_error());
            return 1;
        }

        ASTPrinter printer;
        for (auto& stmt : statements) {
            stmt->accept(printer);
        }
        return 0;
    }

    auto run_ast(const std::string& source) -> int {
        return run_parser(source);
    }
}

auto main(int argc, char** argv) -> int {
    InputParser parser(fs::path(argv[0]).filename().string(),
                   "SLeaf-LLVM - Compiler for SLeaf language");

    parser.add_option({"-v", "--version", "Get version", false, ""});
    parser.add_option({"-h", "--help", "Print help", false, ""});
    parser.add_option({"-c", "--check-utils", "Check required utils", false, ""});
    parser.add_option({"-l", "--lexer", "Run lexer analyzer", false, ""});
    parser.add_option({"-p", "--parser", "Run parser", false, ""});
    parser.add_option({"-a", "--ast", "Run AST printer", false, ""});
    parser.add_option({"-o", "--output", "Output file", true, "file"});

    if (!parser.parse(argc, argv)) {
        for (const auto& error : parser.get_errors()) {
            LOG_ERROR("%s", error.c_str());
        }
        std::cerr << parser.generate_help() << "\n";
        return 1;
    }

    if (parser.has_option("-c")) {
        return check_utils_available() ? 0 : 1;
    }

    if (parser.has_option("-v")) {
        LOG_INFO("Version: %s", VERSION);
        return 0;
    }

    if (parser.has_option("-h") || parser.has_option("--help")) {
        std::cout << parser.generate_help() << "\n";
        return 0;
    }

    if (!check_utils_available()) return 1;

    std::string output_file;
    if (auto output = parser.get_argument("-o")) {
        output_file = *output;
    }

    std::string input_file;
    auto positional = parser.get_positional_args();
    if (!positional.empty()) {
        input_file = positional[0];
    }

    std::string source = read_source(input_file);
    if (source.empty() && input_file.empty()) {
        LOG_ERROR("No input source provided");
        return 1;
    }

    if (parser.has_option("-l")) {
        return run_lexer(source);
    }

    if (parser.has_option("-p")) {
        return run_parser(source);
    }

    if (parser.has_option("-a")) {
        return run_ast(source);
    }

    LOG_INFO("Compilation pipeline not fully implemented yet");
    return 0;
}
