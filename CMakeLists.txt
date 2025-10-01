cmake_minimum_required(VERSION 3.13) # Upped to 3.13 for FetchContent best practices
project(Cerberus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This is required for the FetchContent module
include(FetchContent) 

# --- Platform-Specific Configuration for FBX SDK ---
# This block sets the correct URL and paths based on the operating system.
if(WIN32)
    set(FBX_ARCHIVE_URL "https://github.com/ghere699/Cerberus-dependencies/raw/main/fbx_sdk_windows.zip")
    set(FBX_LIB_SUBDIR "lib") # The path inside the archive is just 'lib/'
    set(FBX_LIB_NAME "libfbxsdk") # Name for find_library and linking
    set(FBX_DLL_NAME "libfbxsdk.dll") # Name for post-build copy
elseif(UNIX AND NOT APPLE)
    set(FBX_ARCHIVE_URL "https://github.com/ghere699/Cerberus-dependencies/raw/main/fbx_sdk_linux.tar.xz")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(FBX_BUILD_CONFIG "debug")
    else()
        set(FBX_BUILD_CONFIG "release") # Default to release
    endif()
    set(FBX_LIB_SUBDIR "lib/gcc/${FBX_BUILD_CONFIG}") # Path inside the archive
    set(FBX_LIB_NAME "fbxsdk") # Name for find_library and linking
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
    message(STATUS "Found Fetched FBX SDK Include Path: ${FBX_INCLUDE_DIR}")
    message(STATUS "Found Fetched FBX SDK Library: ${FBX_LIBRARY}")
endif()

# --- Find System or Vendor Libraries ---
find_package(glfw3 QUIET)
if(glfw3_FOUND)
    message(STATUS "Found system GLFW.")
else()
    message(STATUS "System GLFW not found. Falling back to vendor directory.")
endif()

find_package(glm QUIET)
if(glm_FOUND)
    message(STATUS "Found system GLM.")
else()
    message(STATUS "System GLM not found. Falling back to vendor directory.")
endif()

# --- Aggregate All Source Files ---
# [IMPROVEMENT] Using GLOB_RECURSE is generally discouraged because adding/removing
# files won't automatically trigger a re-configure. Explicitly listing files is better.
# Example of the preferred method:
# set(PROJECT_SOURCES
#     src/main.cpp
#     src/utils/FrameRate.cpp
#     src/core/Scene.cpp
#     # ... list all other cpp files
# )
file(GLOB_RECURSE PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

list(APPEND PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c")
file(GLOB IMGUI_SOURCES "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui*.cpp")
list(APPEND PROJECT_SOURCES ${IMGUI_SOURCES})


# --- Define the Executable Target ---
# [IMPROVEMENT] Set up RPATH so the executable can find the .so at runtime using a relative path.
# This tells the linker to add a runtime search path pointing to the executable's own directory.
if(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif()

# [IMPROVEMENT] Do not list header files in add_executable.
# The GLOB_RECURSE already picked up all the .cpp files from src/, so they are redundant here.
add_executable(Cerberus ${PROJECT_SOURCES})


# --- Configure Target Properties ---
target_include_directories(Cerberus PUBLIC
    ${FBX_INCLUDE_DIR}
    "${CMAKE_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_SOURCE_DIR}/vendor/imgui"
    "${CMAKE_SOURCE_DIR}/vendor/tinygltf"
    "${CMAKE_SOURCE_DIR}/vendor/tinyobj"
    "${CMAKE_SOURCE_DIR}/vendor/stb"
)
if(glfw3_FOUND)
    # find_package for modern glfw3 creates a 'glfw' target that handles its own includes/links
else()
    target_include_directories(Cerberus PUBLIC "${CMAKE_SOURCE_DIR}/vendor/glfw/include")
endif()
if(glm_FOUND)
    # The standard find_package(glm) module sets GLM_INCLUDE_DIRS
    target_include_directories(Cerberus PUBLIC ${GLM_INCLUDE_DIRS})
else()
    target_include_directories(Cerberus PUBLIC "${CMAKE_SOURCE_DIR}/vendor/glm")
endif()

# --- Configure Target Linking ---
# [IMPROVEMENT] Use target_link_directories for the FBX SDK. This allows us to link
# by name (-lfbxsdk) instead of by absolute path, which is more robust.
target_link_directories(Cerberus PRIVATE "${FBX_SDK_DIR}/${FBX_LIB_SUBDIR}")

if(NOT glfw3_FOUND)
    target_link_directories(Cerberus PUBLIC "${CMAKE_SOURCE_DIR}/vendor/glfw/lib-vc2022")
endif()

if(WIN32)
    set(PLATFORM_GL_LIB openGL32.lib)
    set(PLATFORM_GLFW_LIB glfw3.lib)
else()
    set(PLATFORM_GL_LIB GL)
    set(PLATFORM_GLFW_LIB glfw)
endif()

target_link_libraries(Cerberus
    PRIVATE
    # Link GLFW by its target name if found, otherwise by the lib name for vendor version
    ${PLATFORM_GLFW_LIB}
    ${PLATFORM_GL_LIB}
    # [IMPROVEMENT] Link by name now that the directory is set
    ${FBX_LIB_NAME}
)


# --- Post-Build Commands ---
# This command copies the entire resources directory to the output folder.
add_custom_command(TARGET Cerberus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/res" 
    "$<TARGET_FILE_DIR:Cerberus>/res"
    COMMENT "Copying resources to build directory"
)

# This command copies the necessary shared library next to the executable so it can be found at runtime.
if(WIN32)
    add_custom_command(TARGET Cerberus POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${FBX_SDK_DIR}/lib/${FBX_DLL_NAME}"
        "$<TARGET_FILE_DIR:Cerberus>"
        COMMENT "Copying FBX DLL to executable directory"
    )
elseif(UNIX AND NOT APPLE)
    # [IMPROVEMENT] Added post-build step for Linux to copy the shared object.
    # This works in combination with the RPATH setting above.
    add_custom_command(TARGET Cerberus POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${FBX_LIBRARY}" # FBX_LIBRARY holds the full path to the .so file
        "$<TARGET_FILE_DIR:Cerberus>"
        COMMENT "Copying FBX shared library (.so) to executable directory"
    )
endif()