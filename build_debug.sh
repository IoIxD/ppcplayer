echo "=================="
echo "Making PowerPC"
echo "=================="
rm build-PowerPC -rf
mkdir -p build-PowerPC
cd build-PowerPC
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_QJS_LIBC=OFF -DPLATFORM=retroppc -DRETRO68_ROOT=$RETRO68_TOOLCHAIN_PATH -DCMAKE_TOOLCHAIN_FILE=$RETRO68_INSTALL_PATH/cmake/retroppc.toolchain.cmake.in
make
cd .. 
cp build-PowerPC/os9player.bin  ~/.netatalk/Transfer/os9player/os9player.bin
