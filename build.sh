#!/usr/bin/env bash

# Color variables for stylized output
NC='\033[0m' # No color
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
ORANGE='\033[0;33m'

# Functions for formatted output
print_title() {
    echo -e "${YELLOW}======================================================================${NC}"
    echo -e "${YELLOW}$1${NC}"
    echo -e "${YELLOW}======================================================================${NC}"
}

print_header() {
    echo -e "${CYAN}=======================================================${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${CYAN}=======================================================${NC}"
}

print_info() {
    echo -e "${BLUE}[ℹ] $1${NC}"
}

print_success() {
    echo -e "${GREEN}[✓] $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}[⚠] $1${NC}"
}

print_error() {
    echo -e "${RED}[✗] $1${NC}"
}

print_debug() {
    echo -e "${MAGENTA}[🐞] $1${NC}"
}

print_command() {
    echo -e "${ORANGE}\$ $1${NC}"
}

# Function to print the intro with ASCII art                     
print_intro() {
    echo -e "${BLUE}"
    echo -e "${BLUE}███████ ██      ███████  █████  ███████ ${NC}   ${GREEN}SLEAF BUILD${NC}"
    echo -e "${BLUE}██      ██      ██      ██   ██ ██      ${NC}   ${GREEN}Maintained by alexeev-prog${NC}"
    echo -e "${BLUE}███████ ██      █████   ███████ █████   ${NC}   ${CYAN}\033[4mhttps://github.com/alexeev-prog/sleaf-llvm${NC}\033[0m"
    echo -e "${BLUE}     ██ ██      ██      ██   ██ ██      ${NC}   ${CYAN}\033[4mhttps://alexeev-prog.github.io/sleaf-llvm${NC}\033[0m"
    echo -e "${BLUE}███████ ███████ ███████ ██   ██ ██      ${NC}   ${YELLOW}Copyright 2025 (C) alexeev-prog${NC}"
    echo -e "${NC}"
}

print_intro
print_title "SLEAFLLVM Build System"

# Global configuration
BUILD_DIR="build"
DEVELOPER_MODE="OFF"
CONFIG_MODE="Release"
GENERATOR_TYPE=""

# Function to clean build directory
clean_build_dir() {
    if [ -d "$BUILD_DIR" ]; then
        print_warning "Cleaning build directory: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    else
        print_info "Build directory already clean: $BUILD_DIR"
    fi
}

# Function to execute a command and check its success
execute_command() {
    print_command "$1"
    echo
    if ! eval "$1"; then
        echo
        print_error "Operation failed"
        exit 1
    fi
    echo
}

# Function to configure the project with CMake
configure_project() {
    local build_type="$1"
    print_header "Configuring project ($build_type build)"
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR" || { print_error "Failed to enter build directory"; exit 1; }

    local cmake_cmd="cmake .. "
    local cmake_flags=("-Dsleaf-llvm_DEVELOPER_MODE=$DEVELOPER_MODE")

    if [ "$build_type" == "MultiConfig" ]; then
        print_info "Using multi-configuration generator"
        GENERATOR_TYPE="MultiConfig"
        cmake_flags+=("-DCMAKE_CONFIGURATION_TYPES=Debug;Release;RelWithDebInfo")
    else
        print_info "Using single-configuration generator"
        GENERATOR_TYPE="Unix Makefiles"
        cmake_flags+=("-DCMAKE_BUILD_TYPE=$CONFIG_MODE")
    fi

    # Full CMake command
    cmake_cmd+="${cmake_flags[*]}"
    execute_command "$cmake_cmd"

    cd .. # Return to the previous directory
    print_success "Configuration completed!"
}

# Function to build the project
build_project() {
    local build_type="$1"
    print_header "Building project ($build_type build)"
    
    cd "$BUILD_DIR" || { print_error "Build directory not found: $BUILD_DIR"; exit 1; }

    local build_cmd=""
    if [ "$build_type" == "MultiConfig" ]; then
        build_cmd="cmake --build . --config $CONFIG_MODE"
    else
        build_cmd="cmake --build . -j $(nproc)"
    fi

    execute_command "$build_cmd"

    print_success "Build completed successfully!"
    cd .. # Return to the previous directory
}

# Function to run tests
run_tests() {
    print_header "Running test suite"
    
    local test_dir="$BUILD_DIR/bin/test"
    if [ ! -d "$test_dir" ]; then
        print_error "Test directory not found: $test_dir"
        exit 1
    fi

    local tests=()
    while IFS= read -r -d $'\0' test_exec; do
        tests+=("$test_exec")
    done < <(find "$test_dir" -type f -executable -print0)

    if [ ${#tests[@]} -eq 0 ]; then
        print_error "No test executables found in $test_dir"
        exit 1
    fi

    print_info "Found ${#tests[@]} test executables"
    local passed=0
    local failed=0

    for test_exec in "${tests[@]}"; do
        local test_name=$(basename "$test_exec")
        print_info "Running test: $test_name"
        
        echo
        if "$test_exec"; then
            echo
            print_success "$test_name: PASSED"
            ((passed++))
        else
            echo
            print_error "$test_name: FAILED"
            ((failed++))
        fi
    done

    print_header "Test Results"
    echo -e "Total tests: ${#tests[@]}"
    echo -e "${GREEN}Passed: $passed${NC}"
    echo -e "${RED}Failed: $failed${NC}"
    
    if [ "$failed" -gt 0 ]; then
        print_error "Some tests failed!"
        exit 1
    fi

    print_success "All tests passed successfully!"
}

# Function to install the project
install_project() {
    print_header "Installing project"
    
    cd "$BUILD_DIR" || { print_error "Build directory not found: $BUILD_DIR"; exit 1; }

    local install_cmd=""
    if [ "$GENERATOR_TYPE" == "MultiConfig" ]; then
        install_cmd="cmake --install . --config $CONFIG_MODE"
    else
        install_cmd="cmake --install ."
    fi

    execute_command "$install_cmd"

    print_success "Installation completed!"
    cd .. # Return to the previous directory
}

# Usage instructions
usage() {
    echo -e "${YELLOW}Usage:${NC} $0 [command] [option]"
    echo -e "Commands:"
    echo -e "  configure [single|multi]  - Configure project"
    echo -e "  build [single|multi]      - Build project"
    echo -e "  test                      - Run all tests"
    echo -e "  install [single|multi]    - Install project"
    echo -e "  clean                     - Clean build directory"
    echo -e "  all [single|multi]        - Configure + Build"
    echo -e "  alldev [single|multi]     - Dev build + Tests"
    echo -e ""
    echo -e "Options:"
    echo -e "  single   - Single-config generator (default)"
    echo -e "  multi    - Multi-config generator"
    echo -e ""
    echo -e "Examples:"
    echo -e "  ${YELLOW}$0 all single${NC}   - Full build (single config)"
    echo -e "  ${YELLOW}$0 alldev multi${NC} - Developer build with tests"
}

# Main script execution
if [ $# -lt 1 ]; then
    usage
    exit 1
fi

COMMAND=$1
OPTION=${2:-single}  # Default to 'single'

case "$COMMAND" in
    configure)
        configure_project "$OPTION"
        ;;
        
    build)
        build_project "$OPTION"
        ;;
        
    test)
        run_tests
        ;;
        
    install)
        install_project "$OPTION"
        ;;
        
    clean)
        clean_build_dir
        ;;
        
    all)
        configure_project "$OPTION"
        build_project "$OPTION"
        ;;
        
    alldev)
        DEVELOPER_MODE="ON"
        configure_project "$OPTION"
        build_project "$OPTION"
        run_tests
        ;;
        
    *)
        usage
        exit 1
        ;;
esac

print_success "Operation [$COMMAND] completed successfully!"
exit 0
