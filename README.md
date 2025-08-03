# 🌿 sleaf-llvm
<a id="readme-top"></a>

<div align="center">
  <!-- <img src="https://raw.githubusercontent.com/alexeev-prog/sleaf-llvm/refs/heads/main/docs/logo.png" width="250" alt="MorningLang Logo"> -->

  <h3>Basic programming language based on S-expressions made with C++ &amp; LLVM</h3>

  <div>
    <a href="https://marketplace.visualstudio.com/items?itemName=alexeevdev.sleaf-llvmuage-syntax">
      <img src="https://img.shields.io/badge/VSCode-extension?style=for-the-badge&logo=gitbook" alt="Docs">
    </a>
    <a href="https://github.com/alexeev-prog/sleaf-llvm/blob/main/LICENSE">
      <img src="https://img.shields.io/badge/License-GPL_v3-blue?style=for-the-badge&logo=gnu" alt="License">
    </a>
    <a href="https://github.com/alexeev-prog/sleaf-llvm/stargazers">
      <img src="https://img.shields.io/github/stars/alexeev-prog/sleaf-llvm?style=for-the-badge&logo=github" alt="Stars">
    </a>
  </div>
</div>

<br>

<div align="center">
  <img src="https://img.shields.io/github/languages/top/alexeev-prog/sleaf-llvm?style=for-the-badge" alt="Top Language">
  <img src="https://img.shields.io/github/languages/count/alexeev-prog/sleaf-llvm?style=for-the-badge" alt="Language Count">
  <img src="https://img.shields.io/github/license/alexeev-prog/sleaf-llvm?style=for-the-badge" alt="License">
  <img src="https://img.shields.io/github/issues/alexeev-prog/sleaf-llvm?style=for-the-badge&color=critical" alt="Issues">
  <img src="https://img.shields.io/github/last-commit/alexeev-prog/sleaf-llvm?style=for-the-badge" alt="Last Commit">
  <img src="https://img.shields.io/github/contributors/alexeev-prog/sleaf-llvm?style=for-the-badge" alt="Contributors">
</div>

<div align="center" style="margin: 15px 0">
  <img src="https://github.com/alexeev-prog/sleaf-llvm/actions/workflows/static.yml/badge.svg" alt="Static Analysis">
  <img src="https://github.com/alexeev-prog/sleaf-llvm/actions/workflows/ci.yml/badge.svg" alt="CI Build">
</div>

<div align="center">
  <img src="https://raw.githubusercontent.com/alexeev-prog/sleaf-llvm/refs/heads/main/docs/pallet-0.png" width="600" alt="Color Palette">
</div>

> [!CAUTION]
> SLEAF is currently in active alpha development. While core functionality is stable, some advanced features are still evolving. Production use requires thorough testing.

## 🚀 Technical Overview
SLEAF is a statically-typed systems programming language designed for performance-critical applications.
Built on LLVM 19, it combines low-level memory control with expressive S-expression syntax.

## 📦 Installation & Usage

### System Requirements
- LLVM 19 development files
- C++20 compatible clang version
- CPU
- RAM (optional)

### Build Instructions
```bash
# Clone repository with submodules
git clone --recurse-submodules https://github.com/alexeev-prog/sleaf-llvm.git
cd sleaf-llvm

# Build full project
./build.sh all

./build/bin/sleafllvm -h
```

### How To Use
Please, build and install lib. [Instruction here](./BUILDING.md).

You also can read [Hacking](./HACKING.md)

### Core Technical Features
| Feature | Technical Implementation | Performance Impact |
|---------|---------------------------|---------------------|
| **S-expression Syntax** | Lisp-inspired uniform code representation | Reduced cognitive load, enhanced metaprogramming |
| **LLVM19 Backend** | Direct LLVM IR generation via C++ API | Near-native execution speed, advanced optimizations |
| **Cross-Platform** | Single IR → Windows/Linux/macOS binaries | Consistent behavior across platforms |

### Contribution Guidelines
1. **Issue Tracking** - Report bugs via GitHub Issues
2. **Pull Requests** - Follow [CONTRIBUTING.md](CONTRIBUTING.md)
3. **Code Standards** - Adhere to specifications
4. **Performance** - Validate changes with benchmarks
5. **Documentation** - Update relevant documentation

## ⚖️ License
```text
Basic programming language based on S-expressions made with C++ &amp; LLVM
Copyright (C) 2025 Alexeev Bronislav

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```

## SLEAF Manifesto
*"We reject the false choice between performance and expressiveness.
We reject the old methods imposed by backward compatibility with
long-dead legacy products. SLEAF is a new era in researching programming
that combines the simplicity of S-expressions with the functionality
of C++. Thanks to the purity of the project and its versatility,
you can create anything and everything you want."*

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines. Key areas for contribution include:
- Additional test cases for thread-local scenarios
- Performance optimization proposals
- Extended version format support
- IDE integration plugins

## License & Support

This project is licensed under **GNU GPL 3.0 License** - see [LICENSE](https://github.com/alexeev-prog/sleaf-llvm/blob/main/LICENSE). For commercial support and enterprise features, contact [alexeev.dev@mail.ru](mailto:alexeev.dev@mail.ru).

[Explore Documentation](https://alexeev-prog.github.io/sleaf-llvm) |
[Report Issue](https://github.com/alexeev-prog/sleaf-llvm/issues) |
[View Examples](./examples)

---.

<div align="center">
  <br>
  <p>Copyright © 2025 Alexeev Bronislav. Distributed under GNU GPL 3 license</p>
  <a href="#readme-top">↑ Back to Top ↑</a>
  <br>
  <sub>Made with LLVM 19</sub>
</div>
