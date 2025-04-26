# Build

`cmake --preset=default`
`cmake --build build --config Release`

# Run

- note: filepath to the ROM should be provided to stdin.

`./build/Release/GameBoy.exe`

# Dependencies

- SDL2 for application layer stuff (Graphics, Audio, Handling Inputs).
- Catch2 for testing.
