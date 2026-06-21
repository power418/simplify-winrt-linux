cmake_minimum_required(VERSION 3.22)
project("win32-mingw" LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} WIN32
    main.cpp
    resources.rc
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    comctl32
    user32
    ole32
    runtimeobject
)

target_link_options(${PROJECT_NAME} PRIVATE
    -municode
    -mwindows
    -static
)
