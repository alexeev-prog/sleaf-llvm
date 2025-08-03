install(
    TARGETS sleaf-llvm_exe
    RUNTIME COMPONENT sleaf-llvm_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
