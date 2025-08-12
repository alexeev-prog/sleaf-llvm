cmake_minimum_required(VERSION 3.20)

include(cmake/prelude.cmake)

project(
    sleaf-llvm
    VERSION 0.1.0
    DESCRIPTION "System Programming Language in C++ LLVM"
    HOMEPAGE_URL "https://github.com/alexeev-prog/sleaf-llvm"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

if (POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()


# ---- Find dependencies ----

find_package(Boost REQUIRED)
find_package(Clang REQUIRED)
find_package(LLVM REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Catch2 REQUIRED)
find_package(absl REQUIRED)
find_package(libxml2 REQUIRED)

# If you have specific components of LLVM or Clang you need, specify them here
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader TargetParser Target)

# ---- Declare library ----

add_library(
    sleaf-llvm_lib OBJECT
    source/input_parser.cpp
    source/logger.cpp
    source/tracelogger.cpp

    # Lexer-parser
    source/lexer/lexer.cpp
    source/parser/parser.cpp
)

target_include_directories(
    sleaf-llvm_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(sleaf-llvm_lib PUBLIC cxx_std_20)

# ---- Declare executable ----

add_executable(sleaf-llvm_exe source/main.cpp)
add_executable(sleaf-llvm::exe ALIAS sleaf-llvm_exe)

set_property(TARGET sleaf-llvm_exe PROPERTY OUTPUT_NAME sleaf-llvm)

target_compile_features(sleaf-llvm_exe PRIVATE cxx_std_20)

target_link_libraries(sleaf-llvm_exe PRIVATE sleaf-llvm_lib)

target_link_libraries(sleaf-llvm_lib ${llvm_libs} lldELF)
target_link_libraries(sleaf-llvm_lib
    LLVMPasses
    LLVMX86CodeGen
    LLVMX86Desc
    LLVMX86Info
    LLVMOrcJIT
    LLVMExecutionEngine
    LLVMRuntimeDyld
    LLVMMCJIT
    LLVMTarget
)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT sleaf-llvm_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of sleaf-llvm"
  )
endif()

include(cmake/dev-mode.cmake)
