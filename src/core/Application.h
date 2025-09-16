#pragma once

#include "Window.h"
#include "../graphics/Shader.h"
#include "../graphics/Mesh.h"
#include "../graphics/Camera.h"
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

        std::unique_ptr<Window> m_Window;
        std::unique_ptr<Shader> m_Shader;
        std::unique_ptr<Mesh>   m_ModelMesh;
        std::unique_ptr<Camera> m_Camera;

        bool m_IsRunning = true;

        float m_DeltaTime = 0.0f;
        float m_LastFrame = 0.0f;

        float m_LastX = 400, m_LastY = 300;
        bool m_FirstMouse = true;


        std::string m_PathToLoad;
        bool m_limitFps = true;
        float m_maxFps = 144.0f;

        ControlMode m_ControlMode = ControlMode::Camera;
        int m_PrevEscapeKeyState;

    };
}