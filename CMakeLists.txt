cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    sleaf-llvm
    VERSION 0.1.0
    DESCRIPTION "Basic Programming Language in C++ LLVM"
    HOMEPAGE_URL "https://github.com/alexeev-prog/sleaf-llvm"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    sleaf-llvm_lib OBJECT
    source/lib.cpp
)

target_include_directories(
    sleaf-llvm_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(sleaf-llvm_lib PUBLIC cxx_std_20)

# ---- Declare executable ----

add_executable(sleaf-llvm_exe source/main.cpp)
add_executable(sleaf-llvm::exe ALIAS sleaf-llvm_exe)

set_property(TARGET sleaf-llvm_exe PROPERTY OUTPUT_NAME sleaf-llvm)

target_compile_features(sleaf-llvm_exe PRIVATE cxx_std_20)

target_link_libraries(sleaf-llvm_exe PRIVATE sleaf-llvm_lib)

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
