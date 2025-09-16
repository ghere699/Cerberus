# Set the minimum required version of CMake for this project
cmake_minimum_required(VERSION 3.10)

# Define the project name
project(Cerberus)

# Set the C++ standard to C++17 and require it
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# --- Find and Configure External Binary Dependencies ---

# Find the FBX SDK, which is a pre-compiled library
set(FBX_SDK_DIR "${CMAKE_SOURCE_DIR}/vendor/fbxsdk")

# Find the path to the main header file (fbxsdk.h)
find_path(FBX_INCLUDE_DIR fbxsdk.h
    PATHS ${FBX_SDK_DIR}/include
    NO_DEFAULT_PATH
)

# Find the library file (libfbxsdk.lib)
# NOTE: Ensure you have a 'lib' folder inside 'vendor/fbxsdk' containing the .lib file.
find_library(FBX_LIBRARY libfbxsdk.lib
    PATHS ${FBX_SDK_DIR}/lib
    NO_DEFAULT_PATH
)

# Check if we found the SDK and provide useful status or error messages
if(NOT FBX_INCLUDE_DIR OR NOT FBX_LIBRARY)
    message(FATAL_ERROR "FBX SDK not found in ${FBX_SDK_DIR}. 
        Please ensure it contains 'include' and 'lib' subdirectories.")
else()
    message(STATUS "Found FBX SDK Headers: ${FBX_INCLUDE_DIR}")
    message(STATUS "Found FBX SDK Library: ${FBX_LIBRARY}")
endif()


# --- Aggregate All Source Files ---

# 1. Gather all our application's .cpp files recursively from the src/ directory
file(GLOB_RECURSE PROJECT_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# 2. Add the GLAD source file
list(APPEND PROJECT_SOURCES 
    "${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c"
)

# 3. Add all necessary ImGui source files
file(GLOB IMGUI_SOURCES
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_impl_glfw.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_impl_opengl3.cpp"
)
list(APPEND PROJECT_SOURCES ${IMGUI_SOURCES})


# --- Define the Executable Target ---
# Create the final executable named "Cerberus" from all the collected source files
add_executable(Cerberus ${PROJECT_SOURCES} "src/loaders/StlLoader.h" "src/loaders/StlLoader.cpp")


# --- Configure Target Properties (Includes and Linking) ---

# Tell the compiler where to find all necessary header files for our target
target_include_directories(Cerberus PUBLIC
    "${CMAKE_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_SOURCE_DIR}/vendor/glfw/include"
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    "${CMAKE_SOURCE_DIR}/vendor/imgui"
    "${CMAKE_SOURCE_DIR}/vendor/tinygltf"
    ${FBX_INCLUDE_DIR}
)

# Tell the linker where to find the GLFW library files
target_link_directories(Cerberus PUBLIC 
    "${CMAKE_SOURCE_DIR}/vendor/glfw/lib-vc2022" # Ensure this matches your Visual Studio version
)

# Link our executable against all the necessary libraries
target_link_libraries(Cerberus
    PRIVATE
    glfw3.lib
    opengl32.lib # Windows system library for OpenGL
    ${FBX_LIBRARY}
)


# --- Post-Build Commands ---

# 1. Copy the 'res' folder to the executable's output directory after a successful build
add_custom_command(TARGET Cerberus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/res" 
    "$<TARGET_FILE_DIR:Cerberus>/res"
    COMMENT "Copying resources to build directory"
)

# 2. Copy the FBX SDK's DLL to the output directory so the executable can run
# NOTE: Ensure the .dll file is in the 'vendor/fbxsdk/lib' folder alongside the .lib file.
add_custom_command(TARGET Cerberus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${FBX_SDK_DIR}/lib/libfbxsdk.dll"
    "$<TARGET_FILE_DIR:Cerberus>"
    COMMENT "Copying FBX DLL to build directory"
)