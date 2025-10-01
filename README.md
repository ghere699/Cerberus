# Cerberus 3D Model Viewer

Cerberus is a lightweight cross-platform 3D model viewer written in modern C++ and OpenGL. It's designed to be a learning project for modern graphics programming.

## Features

*   **Supported models:**
    *   `.obj`
    *   `.glb`
    *   `.fbx`
    *   `.stl`
*   **Model:**
    *   **Transform:** Translation, rotation, and scaling.
    *   **Material:** Change color, shininess, and other lighting properties.
    *   **Hierarchy:** Select and delete objects from the scene.
*   **Rendering:**
    *   World and model pivot orientation with gizmos.
    *   Rendering modes: Solid, Wireframe, and Points.
    *   Visualization of face and vertex normals.
---

### Building on Linux (e.g., Fedora)
1.  **Clone the Repository:**
    ```bash
    git clone https://github.com/ghere699/Cerberus.git
    cd Cerberus
    ```
2.  **Open the terminal and install these lib:**
    * sudo dnf install glfw-devel
    * sudo dnf install cmake
    * sudo dnf install gcc
	* sudo dnf install g++
	
3.  **Create build folder and compile:**
	*mkdir build
	*cd build
	*cmake .
	*make
	
3.  **Run:**
	* Now you should just do on your terminal ./Cerberus. If the window creation fails, try running with sudo ./Cerberus.
---

### Building on Windows (with Visual Studio)

1.  **Clone the Repository:**
	Make sure that you have cmake installed on your system.
    ```bash
    git clone https://github.com/ghere699/Cerberus.git
    ```
3.  **Open in Visual Studio:**
    *   Launch Visual Studio.
    *   Select "Open a local folder" and choose the `Cerberus` directory.
    *   Visual Studio will automatically detect `CMakeLists.txt` and configure the project.
4.  **Build and Run:**
    *   Now you will just have to compile the program :)
---

## Third-Party Libraries

Cerberus is built using several open source libraries, which are included`vendor/` dir:

*   [**GLFW**]
*   [**GLAD**]
*   [**GLM**]
*   [**ImGui**]
*   [**tinyobjloader**]
*   [**tinygltf**]
*   **Autodesk FBX SDK**
