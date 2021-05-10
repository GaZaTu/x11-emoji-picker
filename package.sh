mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j6
make install DESTDIR=AppDir
~/bin/linuxdeployqt.AppImage AppDir/usr/share/applications/emoji-picker.desktop -appimage -no-translations
cd ..
