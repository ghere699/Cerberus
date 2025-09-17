#pragma once

#include "Window.h"
#include "../graphics/Shader.h"
#include "../graphics/Mesh.h"
#include "../graphics/Camera.h"
#include "../graphics/Gizmo.h"
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
        void Run();

        void QueueModelLoad(const std::string& path);

    private:
        void ProcessInput(float deltaTime);
        void LoadModelFromFile(const std::string& path);

        std::unique_ptr<Window> window_;
        std::unique_ptr<Shader> shader_;
        std::unique_ptr<Mesh>   modelMesh_;
        std::unique_ptr<Camera> camera_;
        std::unique_ptr<Gizmo> gizmo_;

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
        std::string currentModelPath_ = "None";
        int renderMode_ = 0;
        bool enableCulling_ = false;
    };
}