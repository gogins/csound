#!/bin/bash 
pushd `pwd`
cd ~/emsdk
./emsdk activate sdk-incoming-64bit binaryen-master-64bit
source ./emsdk_env.sh
export EMSCRIPTEN_ROOT=$EMSCRIPTEN
echo "Using EMSCRIPTEN_ROOT: $EMSCRIPTEN_ROOT"
#export EMCC_DEBUG=1
popd

mkdir -p build-wasm
cd build-wasm

cmake -DUSE_COMPILER_OPTIMIZATIONS=0 -DWASM=1 -DINIT_STATIC_MODULES=1 -DUSE_DOUBLE=NO -DBUILD_MULTI_CORE=0 -DBUILD_JACK_OPCODES=0 -DEMSCRIPTEN=1 -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN_ROOT/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_MODULE_PATH=$EMSCRIPTEN_ROOT/cmake -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles" -DHAVE_BIG_ENDIAN=0 -DCMAKE_16BIT_TYPE="unsigned short"  -DHAVE_STRTOD_L=0 -DBUILD_STATIC_LIBRARY=YES -DHAVE_ATOMIC_BUILTIN=0 -DHAVE_SPRINTF_L=NO -DUSE_GETTEXT=NO -DLIBSNDFILE_LIBRARY=../deps/libsndfile-1.0.25/libsndfile-wasm.a -DSNDFILE_H_PATH=../deps/libsndfile-1.0.25/src ../../dependencies/csound

emmake make csound-static -j6 VERBOSE=1

emcc -s LINKABLE=1 -s ASSERTIONS=1 -DINIT_STATIC_MODULES=1 ../src/FileList.c -Iinclude -o FileList.bc
emcc -s LINKABLE=1 -s ASSERTIONS=1 -DINIT_STATIC_MODULES=1 ../src/CsoundObj.c -I../../dependencies/csound/include -Iinclude -o CsoundObj.bc
em++ -std=c++11 -s LINKABLE=1 -s ASSERTIONS=1 -DINIT_STATIC_MODULES=1 -I../../dependencies/csound/include ../src/csound_embind.cpp -Iinclude -o csound_web_audio.bc

# Total memory for a WebAssembly module must be a multiple of 64 KB so...
# 1024 * 64 = 65536 is 64 KB
# 65536 * 1024 * 4 is 268435456

# Keep exports in alphabetical order please, to correlate with CsoundObj.js.

em++ -std=c++11 --bind --pre-js ../src/CsoundWebAudio.js -v -O2 -g4 -DINIT_STATIC_MODULES=1 -s WASM=1 -s ASSERTIONS=1 -s "BINARYEN_METHOD='native-wasm'" -s LINKABLE=1 -s RESERVED_FUNCTION_POINTERS=1 -s TOTAL_MEMORY=268435456 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS="['_strlen', '_CsoundObj_closeAudioOut', '_CsoundObj_compileCSD', '_CsoundObj_compileOrc', '_CsoundObj_destroy', '_CsoundObj_evaluateCode', '_CsoundObj_getControlChannel', '_CsoundObj_getInputBuffer', '_CsoundObj_getInputChannelCount', '_CsoundObj_getKsmps', '_CsoundObj_getOutputBuffer', '_CsoundObj_getOutputChannelCount', '_CsoundObj_getTable', '_CsoundObj_getTableLength', '_CsoundObj_getZerodBFS', '_CsoundObj_new', '_CsoundObj_openAudioOut', '_CsoundObj_pause', '_CsoundObj_performKsmps', '_CsoundObj_play', '_CsoundObj_prepareRT', '_CsoundObj_pushMidiMessage', '_CsoundObj_readScore' , '_CsoundObj_render' , '_CsoundObj_reset', '_CsoundObj_setControlChannel', '_CsoundObj_setMidiCallbacks', '_CsoundObj_setOption', '_CsoundObj_setOutputChannelCallback', '_CsoundObj_getScoreTime', '_CsoundObj_setStringChannel', '_CsoundObj_setTable']" CsoundObj.bc FileList.bc csound_web_audio.bc libcsound.a ../deps/libsndfile-1.0.25/libsndfile-wasm.a -o csound_extended.js

cd ..
rm -rf dist-wasm
mkdir dist-wasm
cp build-wasm/libcsound.js dist-wasm/
cp src/*.js dist-wasm/
cp build-wasm/csound_extended.* dist-wasm/
echo "Updating the Csound examples for WebAssembly..."
bash update_example_libs_from_dist_wasm.sh
echo "Creating a release for Csound for WebAssembly..."
bash release-wasm.sh
echo "Finished building Csound for WebAssembly."
ls -ll dist-wasm
