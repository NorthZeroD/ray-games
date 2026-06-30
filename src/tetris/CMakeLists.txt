add_executable(tetris tetris.c)
set_target_properties(tetris PROPERTIES
    C_STANDARD 23
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)
target_compile_options(tetris PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/W4;/permissive->
    $<$<C_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic>
)
target_link_libraries(tetris PRIVATE raylib)
