project('quadcaster', 'cpp', default_options: 'default_library=static')

sources = ['src/main.cpp', 'src/grid_tree.cpp', 'src/grid_map.cpp']
include_dir = include_directories('include')
dependencies = [dependency('sdl3')]

executable(
    'quadcaster',
    sources,
    include_directories: include_dir,
    dependencies: dependencies,
)