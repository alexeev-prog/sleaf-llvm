#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "input_parser.hpp"

#include "absl/strings/match.h"

InputParser::InputParser(std::string program_name, std::string description)
    : m_PROGRAM_NAME(std::move(program_name))
    , m_DESCRIPTION(std::move(description)) {}

auto InputParser::add_option(const Option& opt) -> void {
    if (!opt.short_name.empty() && is_option_registered(opt.short_name)) {
        throw std::invalid_argument("Duplicate short option: " + opt.short_name);
    }

    if (!opt.long_name.empty() && is_option_registered(opt.long_name)) {
        throw std::invalid_argument("Duplicate long option: " + opt.long_name);
    }

    const size_t IDX = m_OPTIONS.size();
    m_OPTIONS.push_back(opt);

    if (!opt.short_name.empty()) {
        m_SHORT_MAP[opt.short_name] = IDX;
    }
    if (!opt.long_name.empty()) {
        m_LONG_MAP[opt.long_name] = IDX;
    }
}

auto InputParser::parse(int argc, char** argv) -> bool {
    reset_state();

    for (int i = 1; i < argc;) {
        const std::string TOKEN = argv[i];
        bool advance_index = true;

        if (is_equals_syntax_option(TOKEN)) {
            handle_equals_syntax(TOKEN);
        } else if (is_regular_option(TOKEN)) {
            advance_index = handle_regular_option(TOKEN, i, argc, argv);
        } else {
            handle_positional_arg(TOKEN);
        }

        if (advance_index) {
            ++i;
        }
    }

    return m_ERRORS.empty();
}

auto InputParser::has_option(const std::string& name) const -> bool {
    const auto IDX = get_option_index(name);
    return IDX && m_PARSED_VALUES.find(*IDX) != m_PARSED_VALUES.end();
}

auto InputParser::get_argument(const std::string& name) const -> std::optional<std::string> {
    const auto IDX = get_option_index(name);
    if (!IDX) {
        return std::nullopt;
    }

    const auto ITERATOR = m_PARSED_VALUES.find(*IDX);
    if (ITERATOR == m_PARSED_VALUES.end()) {
        return std::nullopt;
    }

    return ITERATOR->second;
}

auto InputParser::get_positional_args() const -> const std::vector<std::string>& {
    return m_POSITIONAL_ARGS;
}

auto InputParser::get_errors() const -> const std::vector<std::string>& {
    return m_ERRORS;
}

auto InputParser::generate_help() const -> std::string {
    std::ostringstream oss;
    oss << "Usage: " << m_PROGRAM_NAME << " [options]\n\n";
    oss << m_DESCRIPTION << "\n\n";
    oss << "Options:\n";

    constexpr size_t NAME_WIDTH = 30;

    for (const auto& opt : m_OPTIONS) {
        std::string name_display;
        if (!opt.short_name.empty() && !opt.long_name.empty()) {
            name_display = opt.short_name + ", " + opt.long_name;
        } else {
            name_display = !opt.short_name.empty() ? opt.short_name : opt.long_name;
        }

        if (opt.requires_argument) {
            name_display += " " + opt.arg_placeholder;
        }

        oss << "  " << std::left << std::setw(NAME_WIDTH) << name_display << " " << opt.description << "\n";
    }

    return oss.str();
}

auto InputParser::get_option_index(const std::string& name) const -> std::optional<size_t> {
    // Check exact matches first
    if (auto iter = m_SHORT_MAP.find(name); iter != m_SHORT_MAP.end()) {
        return iter->second;
    }
    if (auto iter = m_LONG_MAP.find(name); iter != m_LONG_MAP.end()) {
        return iter->second;
    }

    // Handle automatic conversion between short and long forms
    if (name.size() > 2 && name.substr(0, 2) == "--") {
        const std::string SHORT_FORM = "-" + name.substr(2);
        if (auto iter = m_SHORT_MAP.find(SHORT_FORM); iter != m_SHORT_MAP.end()) {
            return iter->second;
        }
    } else if (name.size() == 2 && name[0] == '-') {
        const std::string LONGFORM = "--" + name.substr(1);
        if (auto iter = m_LONG_MAP.find(LONGFORM); iter != m_LONG_MAP.end()) {
            return iter->second;
        }
    }

    return std::nullopt;
}

auto InputParser::is_option_registered(const std::string& name) const -> bool {
    return m_SHORT_MAP.find(name) != m_SHORT_MAP.end() || m_LONG_MAP.find(name) != m_LONG_MAP.end();
}

auto InputParser::reset_state() -> void {
    m_PARSED_VALUES.clear();
    m_POSITIONAL_ARGS.clear();
    m_ERRORS.clear();
}

auto InputParser::is_equals_syntax_option(const std::string& token) const -> bool {
    return token.size() >= 2 && token.substr(0, 2) == "--" && absl::StrContains(token, '=');
}

auto InputParser::is_regular_option(const std::string& token) const -> bool {
    return !token.empty() && token[0] == '-';
}

auto InputParser::handle_equals_syntax(const std::string& token) -> void {
    const size_t POS = token.find('=');
    const std::string KEY = token.substr(0, POS);
    const std::string VALUE = token.substr(POS + 1);

    if (const auto ITER = m_LONG_MAP.find(KEY); ITER != m_LONG_MAP.end()) {
        const auto& option = m_OPTIONS[ITER->second];
        if (option.requires_argument) {
            m_PARSED_VALUES[ITER->second] = VALUE;
        } else {
            m_ERRORS.push_back("Option " + KEY + " doesn't accept arguments");
        }
    } else {
        m_ERRORS.push_back("Unknown option: " + KEY);
    }
}

auto InputParser::handle_regular_option(const std::string& token, int& index, int argc, char** argv) -> bool {
    std::optional<size_t> idx;
    bool advance_index = true;

    if (token.size() >= 2 && token.substr(0, 2) == "--") {
        if (auto iter = m_LONG_MAP.find(token); iter != m_LONG_MAP.end()) {
            idx = iter->second;
        }
    } else {
        if (auto iter = m_SHORT_MAP.find(token); iter != m_SHORT_MAP.end()) {
            idx = iter->second;
        }
    }

    if (!idx) {
        m_ERRORS.push_back("Unknown option: " + token);
        return advance_index;
    }

    const auto& option = m_OPTIONS[*idx];
    if (option.requires_argument) {
        if (index + 1 >= argc) {
            m_ERRORS.push_back("Missing argument for: " + token);
        } else {
            m_PARSED_VALUES[*idx] = argv[++index];
            advance_index = false;    // Already advanced index
        }
    } else {
        m_PARSED_VALUES[*idx] = "";
    }

    return advance_index;
}

auto InputParser::handle_positional_arg(const std::string& token) -> void {
    m_POSITIONAL_ARGS.push_back(token);
}
