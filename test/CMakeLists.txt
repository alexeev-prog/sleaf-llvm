# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(sleaf-llvmTests LANGUAGES CXX)

# ---- Tests ----

add_executable(sleaf-llvm_test source/sleaf-llvm_test.cpp)
target_link_libraries(sleaf-llvm_test PRIVATE sleaf-llvm_lib)
target_compile_features(sleaf-llvm_test PRIVATE cxx_std_20)

add_test(NAME sleaf-llvm_test COMMAND sleaf-llvm_test)

# ---- End-of-file commands ----

add_folders(Test)
