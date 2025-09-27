cmake_minimum_required(VERSION 3.10)
project(Cerberus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This is required for the FetchContent module
include(FetchContent) 

# --- Platform-Specific Configuration for FBX SDK ---
# This block sets the correct URL and paths based on the operating system.
if(WIN32)
    # URL to your Windows ZIP file on GitHub Releases
    set(FBX_ARCHIVE_URL "https://github.com/ghere699/Cerberus-dependencies/raw/main/fbx_sdk_windows.zip")
    set(FBX_LIB_SUBDIR "lib") # The path inside the archive is just 'lib/'
    set(FBX_LIB_NAME "libfbxsdk")
    set(FBX_DLL_NAME "libfbxsdk.dll")
elseif(UNIX AND NOT APPLE)
    # URL to your Linux TAR.XZ file on GitHub Releases
    set(FBX_ARCHIVE_URL "https://github.com/ghere699/Cerberus-dependencies/raw/main/fbx_sdk_linux.tar.xz")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(FBX_BUILD_CONFIG "debug")
    else()
        set(FBX_BUILD_CONFIG "release") # Default to release
    endif()
    set(FBX_LIB_SUBDIR "lib/gcc/${FBX_BUILD_CONFIG}") # Path inside the archive
    set(FBX_LIB_NAME "fbxsdk")
else()
    message(FATAL_ERROR "Unsupported platform for FBX SDK.")
endif()


# --- Fetch and Configure FBX SDK ---
FetchContent_Declare(
    fbxsdk_remote
    URL ${FBX_ARCHIVE_URL}
)

FetchContent_GetProperties(fbxsdk_remote)
if(NOT fbxsdk_remote_POPULATED)
    message(STATUS "Fetching FBX SDK from ${FBX_ARCHIVE_URL}...")
    FetchContent_Populate(fbxsdk_remote)
    message(STATUS "FBX SDK fetched and extracted.")
endif()

# The SDK is now available at this path inside your build directory
set(FBX_SDK_DIR ${fbxsdk_remote_SOURCE_DIR})

# Find the header and library files within the fetched directory
find_path(FBX_INCLUDE_DIR fbxsdk.h PATHS ${FBX_SDK_DIR}/include NO_DEFAULT_PATH)
find_library(FBX_LIBRARY NAMES ${FBX_LIB_NAME} PATHS "${FBX_SDK_DIR}/${FBX_LIB_SUBDIR}" NO_DEFAULT_PATH)

if(NOT FBX_INCLUDE_DIR OR NOT FBX_LIBRARY)
    message(FATAL_ERROR "FBX SDK not found after fetch. Check archive structure and paths in CMakeLists.txt.")
else()
    message(STATUS "Found Fetched FBX SDK Library: ${FBX_LIBRARY}")
endif()


# --- Find System (Linux) or Vendor (Windows) Libraries ---
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "System GLFW not found. Falling back to vendor directory.")
    set(PLATFORM_GLFW_LIB glfw3.lib)
else()
    message(STATUS "Found system GLFW.")
    set(PLATFORM_GLFW_LIB glfw)
endif()

find_package(glm QUIET)
if(NOT glm_FOUND)
    message(STATUS "System GLM not found. Falling back to vendor directory.")
    set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/vendor/glm")
endif()


# --- Aggregate All Source Files ---
file(GLOB_RECURSE PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(APPEND PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c")
file(GLOB IMGUI_SOURCES "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui*.cpp")
list(APPEND PROJECT_SOURCES ${IMGUI_SOURCES})


# --- Define the Executable Target ---
add_executable(Cerberus ${PROJECT_SOURCES})


# --- Configure Target Properties ---
target_include_directories(Cerberus PUBLIC
    "${CMAKE_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_SOURCE_DIR}/vendor/imgui"
    "${CMAKE_SOURCE_DIR}/vendor/tinygltf"
    "${CMAKE_SOURCE_DIR}/vendor/tinyobj"
    "${CMAKE_SOURCE_DIR}/vendor/stb"
    ${FBX_INCLUDE_DIR}
)
if(glfw3_FOUND)
    # find_package handles includes for targets like 'glfw' automatically
else()
    target_include_directories(Cerberus PUBLIC "${CMAKE_SOURCE_DIR}/vendor/glfw/include")
endif()
if(glm_FOUND)
    target_include_directories(Cerberus PUBLIC ${GLM_INCLUDE_DIR})
else()
    target_include_directories(Cerberus PUBLIC ${GLM_INCLUDE_DIR})
endif()

if(NOT glfw3_FOUND)
    target_link_directories(Cerberus PUBLIC "${CMAKE_SOURCE_DIR}/vendor/glfw/lib-vc2022")
endif()

if(WIN32)
    set(PLATFORM_GL_LIB openGL32.lib)
else()
    set(PLATFORM_GL_LIB GL)
endif()

target_link_libraries(Cerberus
    PRIVATE
    ${PLATFORM_GLFW_LIB}
    ${PLATFORM_GL_LIB}
    ${FBX_LIBRARY}
)


# --- Post-Build Commands ---
add_custom_command(TARGET Cerberus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/res" 
    "$<TARGET_FILE_DIR:Cerberus>/res"
    COMMENT "Copying resources to build directory"
)

if(WIN32)
    add_custom_command(TARGET Cerberus POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        # Source: The DLL from the fetched content directory
        "${fbxsdk_remote_SOURCE_DIR}/lib/${FBX_DLL_NAME}"
        # Destination: The same directory as the final executable
        "$<TARGET_FILE_DIR:Cerberus>"
        COMMENT "Copying FBX DLL to executable directory"
    )
endif()