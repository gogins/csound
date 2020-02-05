#!/bin/bash
echo "Building the Csound library for WebAssembly..."

pushd `pwd`
cd ~/emsdk
./emsdk activate latest
source ./emsdk_env.sh
echo "Using EMSCRIPTEN_ROOT: $EMSCRIPTEN_ROOT"
# export EMCC_DEBUG=1
popd

cd cmask
rm CMakeCache.txt
emcmake cmake .
emmake make clean
emmake make "VERBOSE=1"
cd ..

mkdir -p build-wasm
cd build-wasm
rm CMakeCache.txt

echo "Packaging some resources..."

python $EMSCRIPTEN_ROOT/tools/file_packager.py csound_samples.data --preload ../../dependencies/csound/samples --js-output=csound_samples.js

echo "Configuring csound-static..."


emcmake cmake -DCMAKE_VERBOSE_MAKEFILE=1 -DBUILD_PLUGINS_DIR="plugins" -DUSE_COMPILER_OPTIMIZATIONS=0 -DWASM=1 -DINIT_STATIC_MODULES=1 -DUSE_DOUBLE=NO -DBUILD_MULTI_CORE=0 -DBUILD_JACK_OPCODES=0 -DEMSCRIPTEN=1 -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN_ROOT/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_MODULE_PATH=$EMSCRIPTEN_ROOT/cmake -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles" -DHAVE_BIG_ENDIAN=0 -DCMAKE_16BIT_TYPE="unsigned short"  -DHAVE_STRTOD_L=0 -DBUILD_STATIC_LIBRARY=YES -DHAVE_ATOMIC_BUILTIN=0 -DHAVE_SPRINTF_L=NO -DUSE_GETTEXT=NO -DLIBSNDFILE_LIBRARY=../deps/libsndfile.a -DSNDFILE_H_PATH=../deps/libsndfile-1.0.25/src ../../dependencies/csound

echo "Making csound-static..."

emmake make csound-static -j6

echo "Compiling FileList..."

emcc -s SAFE_HEAP=0 -s LINKABLE=1 -s ASSERTIONS=1 -s FORCE_FILESYSTEM=1 -DINIT_STATIC_MODULES=1 ../src/FileList.c -Iinclude -o FileList.bc

echo "Compiling CsoundObj..."

emcc -s SAFE_HEAP=0 -s LINKABLE=1 -s ASSERTIONS=1 -s FORCE_FILESYSTEM=1 -DINIT_STATIC_MODULES=1 ../src/CsoundObj.c -I../../dependencies/csound/include -Iinclude -o CsoundObj.bc

echo "Compiling csound_embind..."

em++ -std=c++11 -s SAFE_HEAP=0 -s LINKABLE=1 -s ASSERTIONS=1 -DINIT_STATIC_MODULES=1 -s FORCE_FILESYSTEM=1 -I../../dependencies/csound/include ../src/csound_embind.cpp -Iinclude -o csound_web_audio.bc

# Total memory for a WebAssembly module must be a multiple of 64 KB so...
# 1024 * 64 = 65536 is 64 KB
# 65536 * 1024 * 4 is 268435456

echo "Compiling csound_extended..."

em++ -s SAFE_HEAP=0 -std=c++11 --bind --pre-js ../src/CsoundWebAudio.js --embed-file ../../dependencies/csound/samples@/ -v -O2 -g4 -DINIT_STATIC_MODULES=1 -s WASM=1 -s ASSERTIONS=0  -s "BINARYEN_TRAP_MODE='clamp'" -s FORCE_FILESYSTEM=1 -s "BINARYEN_METHOD='native-wasm'" -s LINKABLE=1 -s RESERVED_FUNCTION_POINTERS=1 -s TOTAL_MEMORY=268435456 -s ALLOW_MEMORY_GROWTH=1 -s BINARYEN_ASYNC_COMPILATION=1 -s MODULARIZE=1 -s "EXPORT_NAME='csound_extended_module'" -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -s FORCE_FILESYSTEM=1 CsoundObj.bc FileList.bc csound_web_audio.bc libcsound.a  ../deps/lib/libsndfile.a ../deps/lib/libogg.a ../deps/lib/libvorbis.a ../deps/lib/libvorbisenc.a ../deps/lib/libFLAC.a -o csound_extended.js

echo "Compiling CsoundAudioProcessor..."

em++ -s SAFE_HEAP=0 -v -O1 -std=c++11 --bind --embed-file ../../dependencies/csound/samples@/ --pre-js ../src/CsoundAudioProcessor_prejs.js --post-js ../src/CsoundAudioProcessor_postjs.js -DINIT_STATIC_MODULES=1 -s FORCE_FILESYSTEM=1 -s WASM=1 -s ASSERTIONS=0 -s "BINARYEN_TRAP_MODE='clamp'" -s "BINARYEN_METHOD='native-wasm'" -s LINKABLE=1 -s RESERVED_FUNCTION_POINTERS=1 -s TOTAL_MEMORY=268435456 -s ALLOW_MEMORY_GROWTH=1 -s BINARYEN_ASYNC_COMPILATION=0 -s NO_EXIT_RUNTIME=0 -s SINGLE_FILE=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -s FORCE_FILESYSTEM=1 csound_web_audio.bc libcsound.a  ../deps/lib/libsndfile.a ../deps/lib/libogg.a ../deps/lib/libvorbis.a ../deps/lib/libvorbisenc.a ../deps/lib/libFLAC.a -o CsoundAudioProcessor.js

cd ..
bash release-wasm.sh
echo "Finished building the Csound library for WebAssembly."
