cmake_minimum_required(VERSION 3.10)
project(amxl LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

include(FetchContent)

#Fetch glfw:
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.4
)

FetchContent_GetProperties(glfw)

if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})

endif()

# Find the Vulkan SDK
find_package(Vulkan REQUIRED)

# Collect all .c files in current directory
file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
set(SOURCES ${C_SOURCES})

# Create the executable target
add_executable(amxl ${SOURCES})
# add_executable(amxl "${CMAKE_SOURCE_DIR}/main.c" "${CMAKE_SOURCE_DIR}/glad.c")

target_compile_options(amxl PRIVATE -fsanitize=address)
target_link_options(amxl PRIVATE -fsanitize=address)

# Add include directories (external libraries)
target_include_directories(amxl
    PRIVATE
        "${CMAKE_SOURCE_DIR}/external/"
)

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(amxl PRIVATE glfw m ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY} "-framework OpenGL")
elseif(WIN32)
    target_link_libraries(amxl PRIVATE Vulkan::Vulkan glfw opengl32 kernel32 user32 gdi32 shell32 m)
else()
    target_link_libraries(amxl PRIVATE Vulkan::Vulkan glfw GL X11 pthread Xi dl m)
endif()

# Have to copy all of the assets to the output directory
add_custom_command(TARGET amxl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data"
        $<TARGET_FILE_DIR:amxl>/data
)
