cmake_minimum_required(VERSION 3.10)
project(Cerberus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Platform-Specific Configuration for FBX SDK ---
if(WIN32)
    set(FBX_PLATFORM_DIR "windows")
    set(FBX_LIB_SUBDIR "") # Looks directly in vendor/fbxsdk/windows/lib
    set(FBX_LIB_NAME "libfbxsdk")
    set(FBX_DLL_NAME "libfbxsdk.dll")

elseif(UNIX AND NOT APPLE)
    set(FBX_PLATFORM_DIR "linux")
    
    # --- THIS IS THE CORRECTED LOGIC FOR LINUX ---
    # Automatically choose debug or release based on the build type
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(FBX_BUILD_CONFIG "debug")
        message(STATUS "Configured for FBX Debug build.")
    else()
        # Default to release for any other configuration (Release, RelWithDebInfo, etc.)
        set(FBX_BUILD_CONFIG "release")
        message(STATUS "Configured for FBX Release build.")
    endif()
    
    # Construct the path based on your directory structure
    set(FBX_LIB_SUBDIR "gcc/${FBX_BUILD_CONFIG}")
    set(FBX_LIB_NAME "fbxsdk")
    # --- END OF CORRECTION ---

else()
    message(FATAL_ERROR "Unsupported platform for FBX SDK.")
endif()

set(FBX_SDK_DIR "${CMAKE_SOURCE_DIR}/vendor/fbxsdk/${FBX_PLATFORM_DIR}")
find_path(FBX_INCLUDE_DIR fbxsdk.h PATHS ${FBX_SDK_DIR}/include NO_DEFAULT_PATH)
find_library(FBX_LIBRARY NAMES ${FBX_LIB_NAME} PATHS "${FBX_SDK_DIR}/lib/${FBX_LIB_SUBDIR}" NO_DEFAULT_PATH)

if(NOT FBX_INCLUDE_DIR OR NOT FBX_LIBRARY)
    message(FATAL_ERROR "FBX SDK not found. Check path: ${FBX_SDK_DIR}/lib/${FBX_LIB_SUBDIR}")
else()
    message(STATUS "Found FBX SDK Headers: ${FBX_INCLUDE_DIR}")
    message(STATUS "Found FBX SDK Library: ${FBX_LIBRARY}")
endif()


# --- Aggregate All Source Files (unchanged) ---
file(GLOB_RECURSE PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(APPEND PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c")
file(GLOB IMGUI_SOURCES "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui*.cpp")
list(APPEND PROJECT_SOURCES ${IMGUI_SOURCES})


# --- Define the Executable Target (unchanged) ---
add_executable(Cerberus ${PROJECT_SOURCES})


# --- Configure Target Properties (Includes and Linking) ---
target_include_directories(Cerberus PUBLIC
    "${CMAKE_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_SOURCE_DIR}/vendor/glfw/include"
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    "${CMAKE_SOURCE_DIR}/vendor/imgui"
    "${CMAKE_SOURCE_DIR}/vendor/tinygltf"
    "${CMAKE_SOURCE_DIR}/vendor/tinyobj"
    ${FBX_INCLUDE_DIR}
)

target_link_directories(Cerberus PUBLIC 
    "${CMAKE_SOURCE_DIR}/vendor/glfw/lib-vc2022"
)

if(WIN32)
    set(PLATFORM_GL_LIB opengl32.lib)
else()
    set(PLATFORM_GL_LIB GL)
endif()

target_link_libraries(Cerberus
    PRIVATE
    glfw3.lib
    ${PLATFORM_GL_LIB}
    ${FBX_LIBRARY}
)


# --- Post-Build Commands (unchanged) ---
add_custom_command(TARGET Cerberus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/res" 
    "$<TARGET_FILE_DIR:Cerberus>/res"
    COMMENT "Copying resources to build directory"
)

if(WIN32)
    add_custom_command(TARGET Cerberus POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${FBX_SDK_DIR}/lib/${FBX_DLL_NAME}"
        "$<TARGET_FILE_DIR:Cerberus>"
        COMMENT "Copying FBX DLL to build directory"
    )
else()
    add_custom_command(TARGET Cerberus POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${FBX_LIBRARY}
        "$<TARGET_FILE_DIR:Cerberus>"
        COMMENT "Copying FBX .so to build directory"
    )
endif()