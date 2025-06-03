#!/bin/bash
mkdir docs
em++ ./src/grid_map.cpp ./src/grid_tree.cpp ./src/main.cpp -I ./include --preload-file maps -sUSE_SDL=3 -Oz -o docs/index.html
