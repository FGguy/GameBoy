tests:
	./build/tests/Release/tests.exe

gen_build: clean
	cmake --preset=default

build_release:
	cmake --build build --config Release

build_debug:
	cmake --build build --config Debug

clean:
	rm -rf ./build