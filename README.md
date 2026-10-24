# NPGR010 - Homeworks

A simple C++17 path tracer used as the starting point for the NPGR010 homeworks.

## Requirements

- A C++17 compiler with OpenMP (GCC, Clang, or MSVC)
- CMake 3.16 or newer

macOS users: Apple's compiler needs a few extra steps, see [BUILD_MACOS.md](BUILD_MACOS.md).

## Building

From the project folder:

```
cmake -S . -B build
cmake --build build
```

The first command configures the project into the `build` subdirectory, the second
one compiles it. After editing the sources, only the second command is needed.

The build is optimized (Release) by default. For a debug build, configure with
`-DCMAKE_BUILD_TYPE=Debug`. With Visual Studio, choose the configuration when
building instead, e.g. `cmake --build build --config Release`.

## Running

```
./build/NPGR010 -h
./build/NPGR010 -s 0 -i 16 -o test.bmp
```

With Visual Studio, the executable is in `build\Release\NPGR010.exe`.

Output can be saved as `.hdr` (default), `.pfm`, or `.bmp`.

## Viewing the results

Use the `.hdr` images. They keep the full range of light values, unlike
`.bmp`, which is clamped and gamma-corrected for quick previews only.
Open them in [tev](https://github.com/Tom94/tev), an HDR image viewer
that lets you change the exposure and compare two images. Downloads for
Windows, Linux and macOS are on its
[releases page](https://github.com/Tom94/tev/releases); on macOS you can
also run `brew install --cask tev`.
