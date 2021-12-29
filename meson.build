project(
    'thundervm', 'c',
    default_options: 'default_library=static'
)

sdl2_dep = dependency('sdl2')
lua_dep = dependency('lua-5.4')

executable(
    'thundervm', 'thundervm.c',
    dependencies: [sdl2_dep, lua_dep]
)
