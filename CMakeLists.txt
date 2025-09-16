cmake_minimum_required(VERSION 3.10)
project(Cerberus)

# --- Project Setup ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Source Files ---
file(GLOB_RECURSE PROJECT_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c"
)

file(GLOB IMGUI_SOURCES
    "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui*.cpp"
)
list(APPEND PROJECT_SOURCES ${IMGUI_SOURCES})

# --- Executable ---
add_executable(Cerberus ${PROJECT_SOURCES})

# --- Dependencies ---
target_include_directories(Cerberus PUBLIC
    "${CMAKE_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_SOURCE_DIR}/vendor/glfw/include"
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    "${CMAKE_SOURCE_DIR}/vendor/imgui"
)

target_link_directories(Cerberus PUBLIC 
    "${CMAKE_SOURCE_DIR}/vendor/glfw/lib-vc2022"
)

target_link_libraries(Cerberus
    PRIVATE
    glfw3.lib
    opengl32.lib
)

# --- Configuration Specific Settings ---
# This section adds the correct flags for Release (optimization) and Debug (debug symbols).
target_compile_definitions(Cerberus PRIVATE
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Debug>:_DEBUG>
)

# MSVC-specific compiler flags
if(MSVC)
    target_compile_options(Cerberus PRIVATE
        $<$<CONFIG:Release>:/O2 /Ot /Oi /Gy /wd4251 /wd4275>
        $<$<CONFIG:Debug>:/Zi /Od>
    )
endif()

# --- Resource Copying ---
add_custom_command(TARGET Cerberus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/res" 
    "$<TARGET_FILE_DIR:Cerberus>/res"
    COMMENT "Copying resources to build directory"
)