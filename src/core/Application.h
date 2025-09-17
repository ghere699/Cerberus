#pragma once

#include "../graphics/Shader.h"
#include "../graphics/Mesh.h"
#include "../graphics/Camera.h"
#include "../graphics/Gizmo.h"
#include "../graphics/Pivot.h"
#include "../graphics/Grid.h"
#include "Window.h"
#include <memory>
#include <string>


enum class ControlMode {
    Camera,
    UI
};

struct SceneObject {
    std::string name;
    std::string filePath;
    std::unique_ptr<Cerberus::Mesh> mesh;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 initialPosition;

    SceneObject() : position(0.0f), rotation(0.0f), scale(1.0f) {}
};

namespace Cerberus {

    class Application {
    public:
        Application();
        ~Application();
        void Run();

        void QueueModelLoad(const std::string& path);

    private:
        void ProcessInput(float deltaTime);
        void LoadModelFromFile(const std::string& path);
        void UIRender();

        std::unique_ptr<Window> window_;
        std::unique_ptr<Shader> shader_;
        std::unique_ptr<Shader> normalsShader_;
        std::unique_ptr<Mesh>   modelMesh_;
        std::unique_ptr<Camera> camera_;
        std::unique_ptr<Gizmo> gizmo_;
        std::unique_ptr<PivotVisualizer> pivotVisualizer_;
        std::unique_ptr<Grid> grid_;

        std::vector<SceneObject> sceneObjects_;
        int selectedObjectIndex_ = -1;

        bool isRunning_ = true;

        float deltaTime_ = 0.0f;
        float lastFrame_ = 0.0f;

        float lastX_ = 400;
        float lastY_ = 300;
        bool firstMouse_ = true;

        std::string pathToLoad_;
        bool limitFps_ = true;
        float maxFps_ = 144.0f;

        ControlMode controlMode_;
        int prevEscapeKeyState_;

        // Model transform variables
        glm::vec3 modelPosition_;
        glm::vec3 modelRotation_;
        glm::vec3 modelScale_;

        bool showModelInfoWindow_ = false;
        int renderMode_ = 0;
        bool enableCulling_ = false;
        bool showFaceNormals_ = false;
        bool showVertexNormals_ = false;
        bool showPivot_ = false;
    };
}