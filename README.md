# Cerberus 3D Model Viewer

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/ghere699/Cerberus)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-blue)](https://github.com/ghere699/Cerberus)
[![License](https://img.shields.io/badge/license-MIT-lightgrey)](LICENSE)

**Cerberus** is a lightweight, cross-platform 3D model viewer written in modern C++ and OpenGL. It's designed to be a learning project for modern graphics programming, focusing on clean code, essential features, and ease of use.


## Features

### 📦 Supported Formats
-   `.obj` (Wavefront OBJ)
-   `.glb` / `.gltf` (GL Transmission Format)
-   `.fbx` (Filmbox)
-   `.stl` (Stereolithography)

### 🛠️ Scene & Model Controls
-   **Transform:** Translate, rotate, and scale models with an intuitive gizmo.
-   **Scene Hierarchy:** View all objects in a tree structure. Select, inspect, and delete individual nodes.
-   **Material Editor:** Tweak material properties like color, shininess, and other lighting parameters on the fly.

### 🎨 Rendering & Visualization
-   **Rendering Modes:** Switch between `Solid`, `Wireframe`, and `Points` to inspect your model's topology.
-   **Normal Visualization:** Display face and vertex normals to debug lighting and geometry issues.
-   **Gizmos:** Clear visual indicators for world orientation and the selected model's pivot point.

## Getting Started

### Prerequisites

Before you begin, ensure you have the following installed on your system:
*   **Git**
*   **CMake** (version 3.16 or later)
*   A C++17 compliant compiler (e.g., **GCC/G++** on Linux, **MSVC** on Windows)

---

### Building on Linux

1.  **Clone the Repository:**
    ```bash
    git clone https://github.com/ghere699/Cerberus.git
    cd Cerberus
    ```

2.  **Install Dependencies:**
    You'll need the development libraries for GLFW. Here are the commands for some popular distributions:

    *   **Debian / Ubuntu:**
        ```bash
        sudo apt update
        sudo apt install build-essential cmake libglfw3-dev
        ```
    *   **Fedora:**
        ```bash
        sudo dnf install gcc-c++ cmake glfw-devel
        ```
    *   **Arch Linux:**
        ```bash
        sudo pacman -S base-devel cmake glfw-x11
        ```

3.  **Configure and Compile:**
    We'll use CMake to generate the build files and then compile the project.
    ```bash
    # Create a build directory
    cmake -B build -DCMAKE_BUILD_TYPE=Release -S .

    # Compile the project
    cmake --build build
    ```

4.  **Run:**
    The executable will be located in the `build` directory.
    ```bash
    ./build/Cerberus
    ```
    > **Note:** If window creation fails, it's likely due to graphics driver or permission issues (e.g., running in a Wayland session that has compatibility problems). Avoid running graphical applications with `sudo` as it is a security risk and masks the underlying problem.

---

### Building on Windows (with Visual Studio 2019/2022)

1.  **Prerequisites:**
    *   Install **Visual Studio 2019/2022** with the **"Desktop development with C++"** workload.
    *   Install **Git** for Windows.
    *   Install **CMake** and ensure it's added to your system's PATH.

2.  **Clone the Repository:**
    Open a terminal (like PowerShell or Git Bash) and run:
    ```bash
    git clone https://github.com/ghere699/Cerberus.git
    ```

3.  **Open in Visual Studio:**
    *   Launch Visual Studio.
    *   From the start screen, select **"Open a local folder"** and navigate to the cloned `Cerberus` directory.
    *   Visual Studio will automatically detect the `CMakeLists.txt` file and start configuring the project. This may take a moment.

4.  **Build and Run:**
    *   Once CMake has finished configuring, you can select the startup item (`Cerberus.exe`) from the dropdown menu in the top toolbar.
    *   Press the green "Run" button or `F5` to build and launch the application.

## Third-Party Libraries

Cerberus is built with the help of these amazing open-source libraries, all included in the `vendor/` directory for convenience:

*   [GLFW](https://www.glfw.org/) - For windowing and input management.
*   [GLAD](https://glad.dav1d.de/) - OpenGL Loading Library.
*   [GLM](https://glm.g-truc.net/0.9.9/index.html) - C++ mathematics library for graphics software.
*   [ImGui](https://github.com/ocornut/imgui) - For the immediate mode graphical user interface.
*   [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) - For loading `.obj` models.
*   [tinygltf](https://github.com/syoyo/tinygltf) - For loading `.gltf` and `.glb` models.
*   **Autodesk FBX SDK** - For loading `.fbx` models.

## Contributing

Contributions are welcome! If you have ideas for new features, bug fixes, or improvements, feel free to open an issue or submit a pull request.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
