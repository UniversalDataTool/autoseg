build/module.js: setup globals.bc
	# -s ENVIRONMENT=web \
	# -Os \
	# -s TOTAL_MEMORY=100MB
	docker run -it -v $(shell pwd):/src seveibar/emscripten em++ \
		-O3 -ffast-math \
		-std=c++1z -I/usr/local/include --bind \
		-o build/module.js ./module.cpp \
		-s MODULARIZE=1 \
		-s SINGLE_FILE=1 \
		-s ENVIRONMENT=web \
		-s ALLOW_MEMORY_GROWTH=1 \
		./globals.bc ./colorspace/ColorSpace.bc ./colorspace/Comparison.bc ./colorspace/Conversion.bc

	# Create separate node version
	docker run -it -v $(shell pwd):/src seveibar/emscripten em++ \
		-O3 -ffast-math \
		-std=c++1z -I/usr/local/include --bind \
		-o build/node.js ./module.cpp \
		-s MODULARIZE=1 \
		-s SINGLE_FILE=1 \
		-s ENVIRONMENT=node \
		-s ALLOW_MEMORY_GROWTH=1 \
		./globals.bc ./colorspace/ColorSpace.bc ./colorspace/Comparison.bc ./colorspace/Conversion.bc

	rm -f test-module/module.js
	rm -f test-module/node.js
	cp build/module.js test-module/module.js
	cp build/node.js test-module/node.js
	cd test-module && npm version patch && npm publish


.PHONY: debug_build/module.js
debug_build/module.js: setup globals.bc
	docker run -it -v $(shell pwd):/src seveibar/emscripten em++ \
		-O3 -ffast-math \
		-std=c++1z -I/usr/local/include --bind -s "EXTRA_EXPORTED_RUNTIME_METHODS=['getValue']" \
		-o build/module.js ./module.cpp \
		-s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s SAFE_HEAP=1 -s DETERMINISTIC=1 -s DEMANGLE_SUPPORT=1 \
		--profiling-funcs -g \
		./globals.bc ./colorspace/ColorSpace.bc ./colorspace/Comparison.bc ./colorspace/Conversion.bc


globals.bc: globals.cpp globals.hpp
	docker run -it -v $(shell pwd):/src seveibar/emscripten em++ -std=c++1z -I/usr/local/include -c ./globals.cpp -o ./globals.bc

test_modulejs: debug_build/module.js
	npx ava --verbose ./tests/modulejs/basic.test.js
	python util/read-bin-image.py tests/modulejs/mask-320x249.bin
	python util/read-bin-image.py tests/modulejs/mask-320x249-polygon-only.bin

assets/orange-320x249.bin:
	mkdir -p assets
	python util/create-bin-image.py ./util/orange-320x249.png ./assets/orange-320x249.bin

assets/image-3024x4032.bin:
	mkdir -p assets
	python util/create-bin-image.py ./util/image-3024x4032.jpg ./assets/image-3024x4032.bin

build/test_super_pixel: setup assets/orange-320x249.bin assets/image-3024x4032.bin globals.o
	clang++ -g -Ofast -o ./build/test_super_pixel	test_super_pixel.cpp globals.o colorspace/Conversion.o colorspace/ColorSpace.o

colorspace:
	mkdir colorspace
	cd colorspace && \
		git init && \
		git remote add origin git@github.com:berendeanicolae/ColorSpace.git && \
		git config core.sparsecheckout true && \
		echo "src" >> .git/info/sparse-checkout && \
		git pull origin master && \
		mv src/* ./  && \
		rmdir src
	cd colorspace && \
		clang++ -Ofast -c ColorSpace.cpp && \
		clang++ -Ofast -c Comparison.cpp && \
		clang++ -Ofast -c Conversion.cpp
	docker run -it -v $(shell pwd)/colorspace:/src seveibar/emscripten em++ -std=c++1z -c ./ColorSpace.cpp -o ./ColorSpace.bc
	docker run -it -v $(shell pwd)/colorspace:/src seveibar/emscripten em++ -std=c++1z -c ./Comparison.cpp -o ./Comparison.bc
	docker run -it -v $(shell pwd)/colorspace:/src seveibar/emscripten em++ -std=c++1z -c ./Conversion.cpp -o ./Conversion.bc

globals.o: globals.cpp globals.hpp
	clang++ -Ofast -c globals.cpp

build/test_min_cut: setup globals.o
	clang++ -Ofast -g -o ./build/test_min_cut test_min_cut.cpp globals.o colorspace/Conversion.o colorspace/ColorSpace.o

build/test_polygon_fill: setup globals.o
	g++ -g -o ./build/test_polygon_fill test_polygon_fill.cpp globals.o colorspace/Conversion.o colorspace/ColorSpace.o

.PHONY: test_polygon_fill
test_polygon_fill: build/test_polygon_fill
	./build/test_polygon_fill
	python util/read-bin-image.py ./polygon_fill-320x249.bin

.PHONY: test_min_cut
test_min_cut: build/test_min_cut
	./build/test_min_cut
	python util/read-bin-image.py ./mincut.bin

.PHONY: test_super_pixel
test_super_pixel: build/test_super_pixel assets/image-3024x4032.bin assets/orange-320x249.bin
	mkdir -p test-outputs
	# valgrind --tool=callgrind ./build/test_super_pixel ./assets/orange-320x249.bin ./test-outputs/orange-320x249-superpixel.bin
	# python util/read-bin-image.py ./test-outputs/orange-320x249-superpixel.bin

	# 118.08 time w/ -O0, 7.70 w/ -Ofast, w/ compute_distance2 6.64
	time ./build/test_super_pixel ./assets/image-3024x4032.bin ./test-outputs/image-3024x4032-superpixel.bin
	python util/read-bin-image.py ./test-outputs/image-3024x4032-superpixel.bin

.PHONY: setup
setup: colorspace
	mkdir -p build

.PHONY: clean
clean:
	rm -f *.o
	rm -f *.bc
	rm -rf test-outputs
	rm -rf build
	rm -rf colorspace
