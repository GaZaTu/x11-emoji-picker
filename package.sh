mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j6
make install DESTDIR=AppDir
~/bin/linuxdeploy.AppImage --appdir AppDir --output appimage -d ../res/emoji-picker.desktop -i ../res/emoji-picker.png
cd ..
