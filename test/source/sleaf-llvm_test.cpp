#include "lib.hpp"

auto main() -> int
{
  auto const lib = library {};

  return lib.name == "sleaf-llvm" ? 0 : 1;
}
