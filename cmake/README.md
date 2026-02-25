# Using mjson with CMake FetchContent

This document describes how to integrate mjson into your CMake-based project using FetchContent.

## Basic Usage

Add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    mjson
    GIT_REPOSITORY https://github.com/cesanta/mjson.git
    GIT_TAG        master  # or specify a specific tag/commit
)

FetchContent_MakeAvailable(mjson)

# Link against your target
target_link_libraries(your_target PRIVATE mjson::mjson)
```

## Configuration Options

mjson provides several options that can be set before calling `FetchContent_MakeAvailable()`:

- `MJSON_ENABLE_PRINT` - Enable printing functionality (default: ON)
- `MJSON_ENABLE_RPC` - Enable RPC functionality (default: ON)
- `MJSON_BUILD_TESTS` - Build unit tests (default: OFF)
- `MJSON_BUILD_EXAMPLES` - Build examples (default: OFF)

### Example with custom options:

```cmake
include(FetchContent)

FetchContent_Declare(
    mjson
    GIT_REPOSITORY https://github.com/cesanta/mjson.git
    GIT_TAG        master
)

# Disable RPC if not needed
set(MJSON_ENABLE_RPC OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(mjson)

target_link_libraries(your_target PRIVATE mjson::mjson)
```

## Using find_package (after installation)

If you've installed mjson on your system, you can use it with `find_package()`:

```cmake
find_package(mjson REQUIRED)
target_link_libraries(your_target PRIVATE mjson::mjson)
```

## Installing mjson

To install mjson:

```bash
cmake -S . -B build
cmake --build build
cmake --install build --prefix /path/to/install
```

## Building Tests

To build and run the unit tests:

```bash
cmake -S . -B build -DMJSON_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```
