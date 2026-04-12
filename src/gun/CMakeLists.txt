SET(EXE_NAME gun)

add_executable(${EXE_NAME})
target_sources(${EXE_NAME} PRIVATE
    gun.cpp
)
target_compile_features(${EXE_NAME} PRIVATE cxx_std_23)
target_compile_options(${EXE_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
target_link_libraries(${EXE_NAME} PRIVATE raylib)

# Web Configurations
# if (${PLATFORM} STREQUAL "Web")
#     set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html") # Tell Emscripten to build an example.html file.
#     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
# endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
# if (APPLE)
#     target_link_libraries(${EXE_NAME} "-framework IOKit")
#     target_link_libraries(${EXE_NAME} "-framework Cocoa")
#     target_link_libraries(${EXE_NAME} "-framework OpenGL")
# endif()

