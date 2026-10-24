cmake_minimum_required(VERSION 3.16)

project(NPGR010 LANGUAGES CXX)

# Default to an optimized build (single-config generators: Makefiles, Ninja)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(NPGR010 src/pg3render.cpp)

target_compile_features(NPGR010 PRIVATE cxx_std_17)

# macOS: Apple clang has no OpenMP runtime; use Homebrew's keg-only libomp
if(APPLE AND NOT OpenMP_ROOT)
    execute_process(
        COMMAND brew --prefix libomp
        OUTPUT_VARIABLE HOMEBREW_LIBOMP
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    if(EXISTS "${HOMEBREW_LIBOMP}")
        set(OpenMP_ROOT "${HOMEBREW_LIBOMP}")
    endif()
endif()

find_package(OpenMP REQUIRED)
target_link_libraries(NPGR010 PRIVATE OpenMP::OpenMP_CXX)
