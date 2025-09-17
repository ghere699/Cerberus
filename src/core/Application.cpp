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
#include <gtc/type_ptr.hpp>

// Static instance pointer for C-style callbacks
static Cerberus::Application* s_Instance = nullptr;

void drop_callback(GLFWwindow* window, int count, const char* paths[])
{
	if (count > 0 && s_Instance) {
		s_Instance->QueueModelLoad(paths[0]);
	}
}


namespace Cerberus {

	Application::Application()
	{
		s_Instance = this;
		prevEscapeKeyState_ = GLFW_RELEASE;

		if (!glfwInit()) {
			throw std::runtime_error("Failed to initialize GLFW");
		}

		window_ = std::make_unique<Window>(800, 600, "Cerberus Project");
		camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
		shader_ = std::make_unique<Shader>("res/shaders/simple.vert", "res/shaders/simple.frag");
		normalsShader_ = std::make_unique<Shader>("res/shaders/normals.vert", "res/shaders/normals.frag");
		gizmo_  = std::make_unique<Gizmo>();
		pivotVisualizer_ = std::make_unique<PivotVisualizer>();
		grid_ = std::make_unique<Grid>();

		glfwSetInputMode(window_->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetDropCallback(window_->GetNativeWindow(), drop_callback);

		LoadModelFromFile("res/models/cube.obj");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window_->GetNativeWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	Application::~Application() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();
	}

	void Application::QueueModelLoad(const std::string& path) {
		std::cout << "File dropped: " << path << std::endl;
		pathToLoad_ = path;
	}

	void Application::LoadModelFromFile(const std::string& path) {
		std::cout << "Attempting to load model: " << path << std::endl;
		std::unique_ptr<Mesh> newMesh = nullptr;
		std::string extension = path.substr(path.find_last_of('.') + 1);

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
			pivotVisualizer_->UpdateSize(newMesh->boundingRadius_);
			modelMesh_ = std::move(newMesh);
			modelPosition_ = glm::vec3(0.0f);
			modelRotation_ = glm::vec3(0.0f);
			modelScale_ = glm::vec3(1.0f);
			currentModelPath_ = path;
			std::cout << "Model transform has been reset." << std::endl;
		}
		else {
			std::cerr << "Failed to load model from path: " << path << std::endl;
		}
	}

	void Application::ProcessInput(float deltaTime) {
		GLFWwindow* window = window_->GetNativeWindow();

		int currentEscapeState = glfwGetKey(window, GLFW_KEY_ESCAPE);
		if (currentEscapeState == GLFW_RELEASE && prevEscapeKeyState_ == GLFW_PRESS) {
			controlMode_ = (controlMode_ == ControlMode::Camera) ? ControlMode::UI : ControlMode::Camera;
		}
		prevEscapeKeyState_ = currentEscapeState;

		if (controlMode_ == ControlMode::Camera) {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera_->ProcessKeyboard(FORWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera_->ProcessKeyboard(BACKWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera_->ProcessKeyboard(LEFT, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera_->ProcessKeyboard(RIGHT, deltaTime);
		}
	}

	void Application::Run() {
		glEnable(GL_DEPTH_TEST);

		while (isRunning_) {
			float currentFrame = static_cast<float>(glfwGetTime());
			deltaTime_ = currentFrame - lastFrame_;
			lastFrame_ = currentFrame;

			if (!pathToLoad_.empty()) {
				LoadModelFromFile(pathToLoad_);
				pathToLoad_.clear();
			}

			if (window_->ShouldClose()) {
				isRunning_ = false;
			}

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ProcessInput(deltaTime_);

			if (controlMode_ == ControlMode::Camera) {
				glfwSetInputMode(window_->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				double xpos, ypos;
				glfwGetCursorPos(window_->GetNativeWindow(), &xpos, &ypos);
				if (firstMouse_) {
					lastX_ = xpos; lastY_ = ypos; firstMouse_ = false;
				}
				float xoffset = xpos - lastX_;
				float yoffset = lastY_ - ypos;
				lastX_ = xpos; lastY_ = ypos;
				camera_->ProcessMouseMovement(xoffset, yoffset);
			}
			else {
				glfwSetInputMode(window_->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				firstMouse_ = true;
			}

			{
				if (ImGui::BeginMainMenuBar()) {
					if (ImGui::BeginMenu("File")) {
						if (ImGui::MenuItem("Model Info")) {
							showModelInfoWindow_ = true;
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Quit", "ESC")) {
							isRunning_ = false;
						}
						ImGui::EndMenu();
					}
					ImGui::EndMainMenuBar();
				}

				if (showModelInfoWindow_) {
					ImGui::Begin("Model Information", &showModelInfoWindow_);
					if (modelMesh_) {
						ImGui::Text("File Path: %s", currentModelPath_.c_str());
						ImGui::Separator();
						// Use %zu for size_t, which is the type of vector::size()
						ImGui::Text("Vertex Count: %zu", modelMesh_->m_Vertices.size());
						ImGui::Text("Index Count: %zu", modelMesh_->m_Indices.size());
						ImGui::Text("Triangle Count: %zu", modelMesh_->m_Indices.size() / 3);
						ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera_->Position.x, camera_->Position.y, camera_->Position.z);
					}
					else {
						ImGui::Text("No model is currently loaded.");
					}
					ImGui::End();
				}

			}

			{
				ImGui::Begin("Cerberus");
				ImGui::Separator();
				ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
				ImGui::Checkbox("Limit Framerate", &limitFps_);
				if (limitFps_) {
					ImGui::SliderFloat("Max FPS", &maxFps_, 30.0f, 240.0f);
				}
				ImGui::Separator();
				ImGui::Text("Camera Settings");
				ImGui::SliderFloat("Movement Speed", &camera_->MovementSpeed, 1.0f, 100.0f);
				if (camera_->MovementSpeed > 100)
					camera_->MovementSpeed = 100;
				ImGui::Text("Render Settings");
				ImGui::Separator();
				ImGui::RadioButton("Solid", &renderMode_, 0); ImGui::SameLine();
				ImGui::RadioButton("Wireframe", &renderMode_, 1); ImGui::SameLine();
				ImGui::RadioButton("Points", &renderMode_, 2);
				ImGui::Text("View Settings");
				ImGui::Separator();
				ImGui::Checkbox("Enable Culling", &enableCulling_);
				ImGui::Checkbox("Show Face Normals", &showFaceNormals_);
				ImGui::SameLine();
				ImGui::Checkbox("Show Vertex Normals", &showVertexNormals_);
				ImGui::Checkbox("Show Model Pivot", &showPivot_);

				ImGui::Separator();
				ImGui::Text("Model Transform");
				ImGui::DragFloat3("Position", glm::value_ptr(modelPosition_), 0.1f);
				ImGui::SliderFloat3("Rotation (Degrees)", glm::value_ptr(modelRotation_), -180.0f, 180.0f);
				ImGui::DragFloat3("Scale", glm::value_ptr(modelScale_), 0.05f);
				if (ImGui::Button("Reset Transform")) {
					modelPosition_ = glm::vec3(0.0f);
					modelRotation_ = glm::vec3(0.0f);
					modelScale_ = glm::vec3(1.0f);
				}
				ImGui::Separator();
				if (ImGui::Button("Quit Application")) {
					isRunning_ = false;
				}
				ImGui::End();
			}

			float aspectRatio = static_cast<float>(window_->GetWidth()) / static_cast<float>(window_->GetHeight());
			glm::mat4 projection = glm::perspective(glm::radians(camera_->Zoom), aspectRatio, 0.1f, 1000.0f);
			glm::mat4 view = camera_->GetViewMatrix();
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, modelPosition_);
			model = glm::rotate(model, glm::radians(modelRotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(modelRotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(modelRotation_.z), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, modelScale_);

			glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			grid_->Draw(view, projection, camera_->Position);

			if (modelMesh_) {
				if (enableCulling_) glEnable(GL_CULL_FACE);
				else glDisable(GL_CULL_FACE);

				if (renderMode_ == 0) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				else if (renderMode_ == 1) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				else { glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); glPointSize(3.0f); }

				shader_->Use();

				shader_->SetVec3("u_objectColor", 1.0f, 1.0f, 1.0f);
				shader_->SetVec3("u_lightColor", 1.0f, 1.0f, 1.0f);
				shader_->SetVec3("u_lightPos", camera_->Position);
				shader_->SetVec3("u_viewPos", camera_->Position);
				shader_->SetMat4("u_Projection", projection);
				shader_->SetMat4("u_View", view);
				shader_->SetMat4("u_Model", model);

				modelMesh_->Draw(*shader_);
			}

			if (showFaceNormals_ && modelMesh_) {
				normalsShader_->Use();
				normalsShader_->SetMat4("u_Projection", projection);
				normalsShader_->SetMat4("u_View", view);
				normalsShader_->SetMat4("u_Model", model);
				modelMesh_->DrawFaceNormals();
			}

			if (showVertexNormals_ && modelMesh_) {
				normalsShader_->Use();
				normalsShader_->SetMat4("u_Projection", projection);
				normalsShader_->SetMat4("u_View", view);
				normalsShader_->SetMat4("u_Model", model);
				modelMesh_->DrawVertexNormals();
			}

			if (showPivot_ && modelMesh_) {
				pivotVisualizer_->Draw(model, view, projection);
			}


			gizmo_->Draw(view, window_->GetWidth(), window_->GetHeight());

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			if (limitFps_) {
				float frameEndTime = static_cast<float>(glfwGetTime());
				float workTime = frameEndTime - currentFrame;
				float targetFrameTime = 1.0f / maxFps_;
				if (workTime < targetFrameTime) {
					auto sleepDuration = std::chrono::microseconds(static_cast<long long>((targetFrameTime - workTime) * 1000000.0f));
					std::this_thread::sleep_for(sleepDuration);
				}
			}
			window_->SwapBuffersAndPollEvents();
		}
	}
}