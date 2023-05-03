#Build Instructions

Windows:

Prerequisites:
VCPKG

Run the command using cmd to build:
```bash
cmake . -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
```

Open build/RCT2RideGen.sln

Build the project you want to use