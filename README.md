## Build

This first generates build files, then builds using those files.

```
cmake --preset=default
cmake --build build --config Release
```

## Run

note: filepath to the ROM should be provided to stdin.

`./build/Release/GameBoy.exe`

## Dependencies

- Assumes you have vcpkg installed at "C:/vcpkg".
