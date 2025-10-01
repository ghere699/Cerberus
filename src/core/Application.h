#pragma once

#include "../graphics/Shader.h"
#include "../graphics/Mesh.h"
#include "../graphics/Camera.h"
#include "../graphics/Gizmo.h"
#include "../graphics/Pivot.h"
#include "../graphics/Grid.h"
#include "../utils/FrameRate.h"
#include "Scene.h"
#include "Window.h"
#include <memory>
#include <string>


enum class ControlMode {
    Camera,
    UI
};

namespace Cerberus {

    class Application {
    public:
        Application();
        ~Application();

        void QueueModelLoad(const std::string& path);
        void Run();

    private:
        void ProcessInput(float deltaTime);
        void LoadModelFromFile(const std::string& path);
        void LoadDefaultModel();

        void UpdateWindowState();
        void HandleInput();
        void Render();
        void UIRender();
        void CreateSceneObjectFromMesh(std::unique_ptr<Mesh> mesh, const std::string& name, const std::string& path);

        // Core management
        std::unique_ptr<Window> window;
        std::unique_ptr<Shader> shader;
        std::unique_ptr<Shader> normalsShader;
        std::unique_ptr<Mesh>   modelMesh;
        std::unique_ptr<Camera> camera;
        std::unique_ptr<Gizmo> gizmo;
        std::unique_ptr<PivotVisualizer> pivotVisualizer;
        std::unique_ptr<Grid> grid;
        std::unique_ptr<FrameRateLimiter> frameRateLimiter;

        Scene scene;
        int selectedEntity = -1;

        bool isRunning = true;
        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        float lastX = 400;
        float lastY = 300;
        bool firstMouse = true;
        std::string pathToLoad;
        float maxFps = 144.0f;
        ControlMode controlMode;
        int prevEscapeKeyState;
        glm::vec3 modelPosition;
        glm::vec3 modelRotation;
        glm::vec3 modelScale;

        bool deleteRequest = false;
        bool showModelInfoWindow = false;
        int renderMode = 0;
        bool enableCulling = false;
        bool showFaceNormals = false;
        bool showVertexNormals = false;
        bool showPivot = false;
    };
}