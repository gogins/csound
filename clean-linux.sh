#!/bin/bash
cd build-linux
echo "Cleaning all Linux artifacts from `pwd`..."
make clean
rm -R --interactive=once *.*
echo "Finished cleaning all Linux artifacts from `pwd`."
cd ..