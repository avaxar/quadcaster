# Quadcaster

 A Data Structures and Algorithms university course project, demonstrating the use case of a tree (specifically, **quadtrees** [trees with a degree of four]) for two-dimensional raycasting. This project uses SDL3 for windowing and handling user input.

## Controls

By default, the program loads the map from `maps/a.txt`. However, one can provide a command line argument to specify the text file to load.

- **WASD** for camera movement
- **Left Shift** for hastened movement
- **Left** and **Right Arrow Keys** for camera yaw rotation
- **Up** and **Down Arrow Keys** to decrease and increase the camera's field of view

## Building

1. Ensure that your system has a compatible C++ compiler installed.
2. Have a development binaries for [SDL3](https://www.libsdl.org). `sudo apt install libsdl3-dev -y` ought to be available on Ubuntu 25.04.
3. Make sure to have a version of the [Meson](https://mesonbuild.com/) build system. On Ubuntu-based distributions, `sudo apt install meson -y` should suffice.
4. Generate build files to a `builddir`: `meson builddir` at the repository root.
5. Execute the build files. For Ninja, `cd builddir && ninja`.
