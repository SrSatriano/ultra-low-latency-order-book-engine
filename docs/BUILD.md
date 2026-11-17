# Build

Requisitos: CMake 3.20+ e um compilador C++20 (GCC 11+, Clang 14+ ou MSVC 2022).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Com sanitizers (GCC/Clang):

```bash
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DOB_SANITIZE=ON
cmake --build build-asan -j && ctest --test-dir build-asan
```

Windows (Developer PowerShell):

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release
```
