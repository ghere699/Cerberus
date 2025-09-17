cmake_minimum_required(VERSION 3.10)


project(Cerberus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(FBX_SDK_DIR "${CMAKE_SOURCE_DIR}/vendor/fbxsdk")

find_path(FBX_INCLUDE_DIR fbxsdk.h
    PATHS ${FBX_SDK_DIR}/include
    NO_DEFAULT_PATH
)

find_library(FBX_LIBRARY libfbxsdk.lib
    PATHS ${FBX_SDK_DIR}/lib
    NO_DEFAULT_PATH
)

if(NOT FBX_INCLUDE_DIR OR NOT FBX_LIBRARY)
    message(FATAL_ERROR "FBX SDK not found in ${FBX_SDK_DIR}. 
        Please ensure it contains 'include' and 'lib' subdirectories.")
else()
    message(STATUS "Found FBX SDK Headers: ${FBX_INCLUDE_DIR}")
    message(STATUS "Found FBX SDK Library: ${FBX_LIBRARY}")
endif()


file(GLOB_RECURSE PROJECT_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

list(APPEND PROJECT_SOURCES 
    "${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c"
)

file(GLOB IMGUI_SOURCES
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_impl_glfw.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_impl_opengl3.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/imgui/ImGuizmo.cpp"
)
list(APPEND PROJECT_SOURCES ${IMGUI_SOURCES})


add_executable(Cerberus ${PROJECT_SOURCES} "src/loaders/StlLoader.h" "src/loaders/StlLoader.cpp"     "src/graphics/Gizmo.h" "src/graphics/Gizmo.cpp" "src/graphics/Pivot.h" "src/graphics/Pivot.cpp" "src/graphics/Grid.h" "src/graphics/Grid.cpp")


target_include_directories(Cerberus PUBLIC
    "${CMAKE_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_SOURCE_DIR}/vendor/glfw/include"
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    "${CMAKE_SOURCE_DIR}/vendor/imgui"
    "${CMAKE_SOURCE_DIR}/vendor/tinygltf"
    ${FBX_INCLUDE_DIR}
)

target_link_directories(Cerberus PUBLIC 
    "${CMAKE_SOURCE_DIR}/vendor/glfw/lib-vc2022" 
)

target_link_libraries(Cerberus
    PRIVATE
    glfw3.lib
    opengl32.lib 
    ${FBX_LIBRARY}
)

add_custom_command(TARGET Cerberus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/res" 
    "$<TARGET_FILE_DIR:Cerberus>/res"
    COMMENT "Copying resources to build directory"
)

add_custom_command(TARGET Cerberus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${FBX_SDK_DIR}/lib/libfbxsdk.dll"
    "$<TARGET_FILE_DIR:Cerberus>"
    COMMENT "Copying FBX DLL to build directory"
)