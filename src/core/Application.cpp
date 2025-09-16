#include "Application.h"
#include "../loaders/ObjLoader.h"
#include "../loaders/GltfLoader.h"
#include "../loaders/FbxLoader.h"
#include "../loaders/StlLoader.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <chrono>
#include <thread>

static Cerberus::Application* s_Instance = nullptr;

void drop_callback(GLFWwindow* window, int count, const char* paths[])
{
	if (count > 0 && s_Instance) {
		s_Instance->QueueModelLoad(paths[0]);
	}
}


namespace Cerberus {

	Application::Application() {
		s_Instance = this;
        m_PrevEscapeKeyState = GLFW_RELEASE;

		if (!glfwInit()) {
			throw std::runtime_error("Failed to initialize GLFW");
		}

		m_Window = std::make_unique<Window>(800, 600, "Cerberus Project");

		glfwSetInputMode(m_Window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glfwSetDropCallback(m_Window->GetNativeWindow(), drop_callback);
		m_Camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
		LoadModelFromFile("res/models/cube.obj");
		m_Shader = std::make_unique<Shader>("res/shaders/simple.vert", "res/shaders/simple.frag");

		// --- IMGUI INITIALIZATION ---
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		// Initialize backends
		ImGui_ImplGlfw_InitForOpenGL(m_Window->GetNativeWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	Application::~Application() {
		// --- IMGUI SHUTDOWN ---
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwTerminate();
	}

	void Application::QueueModelLoad(const std::string& path) {
		std::cout << "File dropped: " << path << std::endl;
		m_PathToLoad = path;
	}

    void Application::LoadModelFromFile(const std::string& path) {
        std::cout << "Attempting to load model: " << path << std::endl;

        std::unique_ptr<Mesh> newMesh = nullptr;
        std::string extension = path.substr(path.find_last_of(".") + 1);

        if (extension == "obj") {
            newMesh = ObjLoader::LoadModel(path);
        }
        else if (extension == "glb") {
            newMesh = GltfLoader::LoadModel(path);
        }
        else if (extension == "fbx") {
            newMesh = FbxLoader::LoadModel(path);
        }
        else if (extension == "stl") {
            newMesh = StlLoader::LoadModel(path);
        }
        else {
            std::cerr << "Unsupported file format: " << extension << std::endl;
            return;
        }

        if (newMesh) {
            m_ModelMesh = std::move(newMesh);
        }
        else {
            std::cerr << "Failed to load model from path: " << path << std::endl;
        }
    }

	void Application::ProcessInput(float deltaTime) {
		GLFWwindow* window = m_Window->GetNativeWindow();

		// --- ESC Key Toggles Control Mode ---
		int currentEscapeState = glfwGetKey(window, GLFW_KEY_ESCAPE);
		if (currentEscapeState == GLFW_RELEASE && m_PrevEscapeKeyState == GLFW_PRESS) {
			if (m_ControlMode == ControlMode::Camera) {
				m_ControlMode = ControlMode::UI;
			}
			else {
				m_ControlMode = ControlMode::Camera;
			}
		}
		m_PrevEscapeKeyState = currentEscapeState;

		// --- WASD Movement only in Camera Mode ---
		if (m_ControlMode == ControlMode::Camera) {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) m_Camera->ProcessKeyboard(FORWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) m_Camera->ProcessKeyboard(BACKWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) m_Camera->ProcessKeyboard(LEFT, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) m_Camera->ProcessKeyboard(RIGHT, deltaTime);
		}
	}

    void Application::Run() {
        glEnable(GL_DEPTH_TEST);

        while (m_IsRunning) {
            float currentFrame = static_cast<float>(glfwGetTime());
            m_DeltaTime = currentFrame - m_LastFrame;
            m_LastFrame = currentFrame;

            // --- Check for new model to load ---
            if (!m_PathToLoad.empty()) {
                LoadModelFromFile(m_PathToLoad);
                m_PathToLoad.clear();
            }

            if (m_Window->ShouldClose()) {
                m_IsRunning = false;
            }

            // --- Start new ImGui frame ---
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ProcessInput(m_DeltaTime); // Handles ESC toggle and WASD movement

            if (m_ControlMode == ControlMode::Camera) {
                // Camera Mode: Hide and lock cursor for camera control
                glfwSetInputMode(m_Window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                double xpos, ypos;
                glfwGetCursorPos(m_Window->GetNativeWindow(), &xpos, &ypos);

                if (m_FirstMouse) {
                    m_LastX = xpos;
                    m_LastY = ypos;
                    m_FirstMouse = false;
                }

                float xoffset = xpos - m_LastX;
                float yoffset = m_LastY - ypos; // reversed since y-coordinates go from bottom to top
                m_LastX = xpos;
                m_LastY = ypos;

                m_Camera->ProcessMouseMovement(xoffset, yoffset);
            }
            else { 
                glfwSetInputMode(m_Window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                m_FirstMouse = true;
            }

            {
                ImGui::Begin("Cerberus Controller");
                ImGui::Separator();

                ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
                ImGui::Checkbox("Limit Framerate", &m_limitFps);
                // FPS slider ...

                ImGui::Separator();
                ImGui::Text("Camera Settings");
                ImGui::SliderFloat("Movement Speed", &m_Camera->MovementSpeed, 1.0f, 20.0f);
                ImGui::Separator();

                ImGui::Text("Model Transform");
                ImGui::DragFloat3("Position", glm::value_ptr(m_ModelPosition), 0.1f);
                ImGui::SliderFloat3("Rotation (Degrees)", glm::value_ptr(m_ModelRotation), -180.0f, 180.0f);
                ImGui::DragFloat3("Scale", glm::value_ptr(m_ModelScale), 0.05f);

                if (ImGui::Button("Reset Transform")) {
                    m_ModelPosition = glm::vec3(0.0f);
                    m_ModelRotation = glm::vec3(0.0f);
                    m_ModelScale = glm::vec3(1.0f);
                }

                ImGui::Separator();
                if (ImGui::Button("Quit Application")) {
                    m_IsRunning = false;
                }
                ImGui::End();
            }

            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            m_Shader->Use();

            // Set lighting and camera uniforms
            m_Shader->SetVec3("u_objectColor", 1.0f, 1.0f, 1.0f);
            m_Shader->SetVec3("u_lightColor", 1.0f, 1.0f, 1.0f);
            m_Shader->SetVec3("u_lightPos", m_Camera->Position);
            m_Shader->SetVec3("u_viewPos", m_Camera->Position);

            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, m_ModelPosition);
            model = glm::rotate(model, glm::radians(m_ModelRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(m_ModelRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(m_ModelRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, m_ModelScale);

            // Set matrix uniforms
            float aspectRatio = static_cast<float>(m_Window->GetWidth()) / static_cast<float>(m_Window->GetHeight());
            glm::mat4 projection = glm::perspective(glm::radians(m_Camera->Zoom), aspectRatio, 0.1f, 1000.0f);
            glm::mat4 view = m_Camera->GetViewMatrix();

            m_Shader->SetMat4("u_Projection", projection);
            m_Shader->SetMat4("u_View", view);
            m_Shader->SetMat4("u_Model", model);

            if (m_ModelMesh) {
                m_ModelMesh->Draw(*m_Shader);
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (m_limitFps) {
                float frameEndTime = static_cast<float>(glfwGetTime());
                float workTime = frameEndTime - currentFrame;
                float targetFrameTime = 1.0f / m_maxFps;
                if (workTime < targetFrameTime) {
                    auto sleepDuration = std::chrono::microseconds(static_cast<long long>((targetFrameTime - workTime) * 1000000.0f));
                    std::this_thread::sleep_for(sleepDuration);
                }
            }

            m_Window->SwapBuffersAndPollEvents();
        }
	}
}