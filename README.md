# Quadcaster

 A Data Structures and Algorithms university course project, demonstrating the use case of a tree (specifically, **quadtrees** [trees with a degree of four]) for two-dimensional raycasting. This project uses SDL3 for windowing and handling user input.

## Building

1. Ensure that your system has a supported C++ compiler installed.
2. Have a development binaries for SDL3. `sudo apt install libsdl3-dev -y` ought to be available on Ubuntu 25.04.
3. Make sure to have a version of the [Meson](https://mesonbuild.com/) build system. On Ubuntu-based distributions, `sudo apt install meson -y` should suffice.
4. Generate build files to a `builddir`: `meson builddir` at the repository root.
5. Execute the build files. For Ninja, `cd builddir && ninja`.
