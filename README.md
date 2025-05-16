## Build

This first generates build files, then builds using those files.

```
cmake --preset=default
cmake --build build --config Release
```

## Run Emulator

note: filepath to the cartridge ROM should be provided to stdin.

`./build/Release/GameBoy.exe`

## Run Tests

`./build/tests/Release/tests.exe`

## Dependencies

- Assumes you have vcpkg installed at "C:/vcpkg".
