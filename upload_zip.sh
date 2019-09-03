#!/bin/bash
if [ "$1" == "make" ]; then
    echo "make"
    rm -r build lib bin
    mkdir build
    cd build
    cmake ..
    make
    cd ..
    echo "make done"
fi
upload_zip="yangwuuu.zip"
if [ -f $upload_zip ]; then
    rm $upload_zip
fi
chmod +x gameclient.sh
zip -r $upload_zip "bin/ai_yang" "gameclient.sh"

