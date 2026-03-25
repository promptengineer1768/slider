# AI Agent Guidelines

## Project Requirements
- Use C++20
- Follow Google-style C++ formatting and conventions:
  - https://google.github.io/styleguide/cppguide.html
- Keep the code modular, with reusable logic separated from UI code where practical
- Maintain cross-compiler compatibility across MSVC, GCC, and LLVM/Clang
- Prefer designs that can be extended toward cross-platform support on Windows, Linux, and macOS
- Keep security in mind for file handling, export behavior, and external input
- Keep the codebase ready for future internationalization work

## Repository Layout
- `src/` implementation files
- `include/` public headers
- `tests/` unit tests
- `docs/` documentation
- `build/` generated build output
- `debug/` temporary files, logs, and local debug output (tracked directory, contents gitignored)
- `tools/local/` local-only dev tools like act (scripts tracked, downloads gitignored)

**Important**: All temporary files, logs, debug output, and local-only data should be placed in `debug/` directory. Never commit temporary files to other locations.

## Build Requirements
- Use CMake with Ninja as the build system
- Keep compiler warnings high and treat warnings as errors where practical
- Ensure the project can be built through MSVC, GCC, and LLVM/Clang entrypoints

## Engineering Expectations
- Prefer small, testable components
- Add or update tests for parser and exporter behavior when those areas change
- Avoid platform-specific code in portable library modules
- Document meaningful design or build changes in `README.md` or `docs/` when needed

## Pinned Tool Versions

These versions are the highest available across all supported platforms (Ubuntu 24.04 LTS, Homebrew, Chocolatey).
The binding constraint is Ubuntu 24.04 LTS apt packages. All setup scripts and CI must install these versions.

| Tool          | Version      | Ubuntu 24.04 (apt) | Homebrew   | Chocolatey |
|---------------|-------------|--------------------|------------|------------|
| CMake | >= 3.28 | 3.28.3 | 4.3.0 | 4.3.0 |
| Ninja         | >= 1.11      | 1.11.1             | 1.13.2     | 1.13.2     |
| Git           | >= 2.43      | 2.43.0             | 2.53.0     | 2.53.0     |
| GCC           | >= 13        | 13.2.0             | 15.2.0     | n/a        |
| Clang | >= 19 | 19.x (clang-19) | 22.1.1 | latest |
| clang-tidy | >= 19 | 19.x (clang-tools-19) | 22.1.1 | latest |
| clang-format | >= 19 | 19.1.1 (clang-format-19) | 22.1.1 | latest |
| wxWidgets     | >= 3.2       | 3.2.x              | 3.2.x      | 3.3.1      |
| GTest         | >= 1.14.0    | 1.14.0             | 1.17.x     | 1.17.x     |

CMake 4.x is supported. The CMakeLists.txt uses `cmake_minimum_required(VERSION 3.28)`.

GTest is pinned to >= 1.14.0 to match Ubuntu 24.04's apt package (libgtest-dev 1.14.0).

On Ubuntu 24.04, clang/g++ are versioned (clang-19, g++-13). CI workflows must pass
`-DCMAKE_C_COMPILER=clang-19 -DCMAKE_CXX_COMPILER=clang++-19` for clang builds,
and `-DCMAKE_C_COMPILER=gcc-13 -DCMAKE_CXX_COMPILER=g++-13` for gcc builds.

## Controlled Build Environment

The bootstrap scripts install tools to known locations via winget but do NOT modify
the current session's PATH. This avoids breaking the user's existing environment.

The build scripts construct a controlled PATH only for the cmake subprocess:
- A temporary batch file is created that calls vcvarsall.bat and prepends our tool directories
- This subprocess gets the controlled PATH; the user's shell remains untouched
- MSYS2 builds use the MSYS2 bash which manages its own PATH internally

## Build Commands

### Windows
Use the prepared batch files to build and test:

```
build-msvc-debug.bat      # Build MSVC debug
build-msvc-release.bat    # Build MSVC release
build-clang-debug.bat     # Build clang-cl debug
build-gcc-debug.bat       # Build GCC (MSYS2) debug
test-all.bat              # Run all tests
package-windows.bat       # Create installer (requires release build)
```

These batch files:
1. Bootstrap the build environment (install VS Build Tools if needed)
2. Setup vcpkg and dependencies
3. Configure, build, and test using CMake presets

Do NOT run cmake directly. Always use the batch files to ensure proper environment setup.

### Linux
```
cmake --preset linux-clang-debug -DCMAKE_C_COMPILER=clang-19 -DCMAKE_CXX_COMPILER=clang++-19
cmake --build --preset build-linux-clang-debug
ctest --preset test-linux-clang-debug
```

### macOS
```
cmake --preset macos-clang-debug
cmake --build --preset build-macos-clang-debug
ctest --preset test-macos-clang-debug
```

### Local CI Simulation (optional)
To simulate GitHub Actions locally on Windows, run `tools\local\setup-act.bat`.
This installs `act` to `tools/local/` and requires Docker Desktop.
Only Linux/macOS jobs can be simulated.
This tool is local-only and not part of the official build system.

### Checking CI Results
GitHub workflow results can be fetched using the GitHub API via webfetch:
- List recent runs: `https://api.github.com/repos/{owner}/{repo}/actions/runs?per_page=5`
- Get job details: `https://api.github.com/repos/{owner}/{repo}/actions/runs/{run_id}/jobs`
- View workflow HTML: `https://github.com/{owner}/{repo}/actions/runs/{run_id}`
