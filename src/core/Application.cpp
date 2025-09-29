#include "Application.h"
#include "../loaders/ObjLoader.h"
#include "../loaders/GltfLoader.h"
#include "../loaders/FbxLoader.h"
#include "../loaders/StlLoader.h"
#include "../../res/models/DefaultAssets.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>


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
		gizmo_ = std::make_unique<Gizmo>();
		pivotVisualizer_ = std::make_unique<PivotVisualizer>();
		grid_ = std::make_unique<Grid>();
		frameRateLimiter_ = std::make_unique<FrameRateLimiter>(maxFps_);

		glfwSetInputMode(window_->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetDropCallback(window_->GetNativeWindow(), drop_callback);

		LoadDefaultModel();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(window_->GetNativeWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	Application::~Application() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();
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
			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera_->ProcessKeyboard(UP, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera_->ProcessKeyboard(DOWN, deltaTime);
		}
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
			SceneObject newObject;
			newObject.mesh = std::move(newMesh);
			newObject.name = path.substr(path.find_last_of("/\\") + 1);
			newObject.filePath = path;
			glm::vec3 spawnPosition(0.0f);
			if (!sceneObjects_.empty()) {
				const SceneObject& lastObject = sceneObjects_.back();
				float padding = 2.0f;
				spawnPosition.x = lastObject.position.x +
					lastObject.mesh->boundingRadius_ * lastObject.scale.x +
					newObject.mesh->boundingRadius_ * newObject.scale.x +
					padding;
			}

			newObject.position = spawnPosition;
			newObject.initialPosition = spawnPosition;

			sceneObjects_.push_back(std::move(newObject));
			selectedObjectIndex_ = sceneObjects_.size() - 1;

			pivotVisualizer_->UpdateSize(sceneObjects_.back().mesh->boundingRadius_);

			std::cout << "Successfully added " << sceneObjects_.back().name << " to the scene." << std::endl;
		}

		else {
			std::cerr << "Failed to load model from path: " << path << std::endl;
		}
	}

	void Application::LoadDefaultModel() {
		std::cout << "Loading default model from memory.." << std::endl;

		const char* data = Cerberus::DefaultAssets::CubeObj.c_str();
		size_t size = Cerberus::DefaultAssets::CubeObj.length();

		std::unique_ptr<Mesh> defaultMesh = ObjLoader::LoadModelFromMemory(data, size);

		if (defaultMesh) {
			SceneObject newObject;
			newObject.mesh = std::move(defaultMesh);
			newObject.name = "Default Cube";
			newObject.filePath = "Internal Memory";

			sceneObjects_.push_back(std::move(newObject));
			selectedObjectIndex_ = 0;

			pivotVisualizer_->UpdateSize(sceneObjects_.back().mesh->boundingRadius_);
			std::cout << "Default model loaded successfully." << std::endl;
		}
		else {
			std::cerr << "Failed to load default model from memory." << std::endl;
		}
	}

	void Application::UpdateWindowState() {
		if (!pathToLoad_.empty()) {
			LoadModelFromFile(pathToLoad_);
			pathToLoad_.clear();
		}

		if (window_->ShouldClose()) {
			isRunning_ = false;
		}
	}

	void Application::HandleInput() {
		static ImGuiIO& io = ImGui::GetIO();
		ProcessInput(deltaTime_);

		bool isCameraActive = (controlMode_ == ControlMode::Camera) && !io.WantCaptureMouse;

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
	}

	void Application::Render() {
		float aspectRatio = static_cast<float>(window_->GetWidth()) / static_cast<float>(window_->GetHeight());
		glm::mat4 projection = glm::perspective(glm::radians(camera_->Zoom), aspectRatio, 0.1f, 1000.0f);
		glm::mat4 view = camera_->GetViewMatrix();

		glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		grid_->Draw(view, projection, camera_->Position);

		for (int i = 0; i < sceneObjects_.size(); ++i) {
			const auto& sceneObject = sceneObjects_[i];

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, sceneObject.position);
			model = glm::rotate(model, glm::radians(sceneObject.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(sceneObject.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(sceneObject.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, sceneObject.scale);

			if (i == selectedObjectIndex_) {
				if (enableCulling_) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
				if (renderMode_ == 0) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				else if (renderMode_ == 1) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				else { glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); glPointSize(3.0f); }
			}
			else {
				glDisable(GL_CULL_FACE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			shader_->Use();
			shader_->SetVec3("u_objectColor", sceneObject.color);
			shader_->SetFloat("u_shininess", sceneObject.shininess);
			shader_->SetFloat("u_specularStrength", sceneObject.specularStrength);
			shader_->SetFloat("u_ambientStrength", sceneObject.ambientStrength);

			shader_->SetVec3("u_lightColor", 1.0f, 1.0f, 1.0f);
			shader_->SetVec3("u_lightPos", camera_->Position);
			shader_->SetVec3("u_viewPos", camera_->Position);
			shader_->SetMat4("u_Model", model);
			shader_->SetMat4("u_View", view);
			shader_->SetMat4("u_Projection", projection);
			sceneObject.mesh->Draw(*shader_);
		}

		if (isObjectValid()) {
			const SceneObject& selectedObject = sceneObjects_[selectedObjectIndex_];

			glm::mat4 selectedModelMatrix = glm::mat4(1.0f);
			selectedModelMatrix = glm::translate(selectedModelMatrix, selectedObject.position);
			selectedModelMatrix = glm::rotate(selectedModelMatrix, glm::radians(selectedObject.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			selectedModelMatrix = glm::rotate(selectedModelMatrix, glm::radians(selectedObject.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			selectedModelMatrix = glm::rotate(selectedModelMatrix, glm::radians(selectedObject.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
			selectedModelMatrix = glm::scale(selectedModelMatrix, selectedObject.scale);

			if (showPivot_) {
				pivotVisualizer_->Draw(selectedModelMatrix, view, projection);
			}
			if (showFaceNormals_) {
				normalsShader_->Use();
				normalsShader_->SetMat4("u_Model", selectedModelMatrix);
				normalsShader_->SetMat4("u_View", view);
				normalsShader_->SetMat4("u_Projection", projection);
				selectedObject.mesh->DrawFaceNormals();
			}
			if (showVertexNormals_) {
				normalsShader_->Use();
				normalsShader_->SetMat4("u_Model", selectedModelMatrix);
				normalsShader_->SetMat4("u_View", view);
				normalsShader_->SetMat4("u_Projection", projection);
				selectedObject.mesh->DrawVertexNormals();
			}
		}

		gizmo_->Draw(view, window_->GetWidth(), window_->GetHeight());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Application::UIRender() {
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
			if (ImGui::Begin("Model Information", &showModelInfoWindow_)) {
				if (isObjectValid()) {
					const SceneObject& selectedObject = sceneObjects_[selectedObjectIndex_];

					ImGui::Text("File Path: %s", selectedObject.filePath.c_str());
					ImGui::Separator();
					ImGui::Text("Vertex Count: %zu", selectedObject.mesh->m_Vertices.size());
					ImGui::Text("Index Count: %zu", selectedObject.mesh->m_Indices.size());
					ImGui::Text("Triangle Count: %zu", selectedObject.mesh->m_Indices.size() / 3);
				}
				else {
					ImGui::Text("No object selected.");
				}

				ImGui::Separator();
				ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera_->Position.x, camera_->Position.y, camera_->Position.z);

			}
			ImGui::End();
		}


		if (ImGui::Begin("Cerberus")) {
			if (ImGui::BeginChild("Hierarchy", ImVec2(ImGui::GetContentRegionAvail().x, 200), true)) {
				if (sceneObjects_.empty()) {
					ImGui::Text("No objects in scene.");
				}
				else {
					for (int i = 0; i < sceneObjects_.size(); ++i) {
						if (ImGui::Selectable(sceneObjects_[i].name.c_str(), selectedObjectIndex_ == i)) {
							selectedObjectIndex_ = i;
							if (selectedObjectIndex_ < sceneObjects_.size()) {
								pivotVisualizer_->UpdateSize(sceneObjects_[i].mesh->boundingRadius_);
							}
						}
					}
				}

				//thx copilot		
				if (deleteRequest && isObjectValid()) {
					sceneObjects_.erase(sceneObjects_.begin() + selectedObjectIndex_);
					if (sceneObjects_.empty()) {
						selectedObjectIndex_ = -1;
					}
					else if (selectedObjectIndex_ >= sceneObjects_.size()) {
						selectedObjectIndex_ = sceneObjects_.size() - 1;
					}
					deleteRequest = false;
				}
			}
			ImGui::EndChild();

			if (ImGui::Button("Destroy", ImVec2(120, 20))) {
				deleteRequest = true;
			}

			ImGui::Separator();
			ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
			float oldMaxFps = maxFps_;
			ImGui::SliderFloat("Max FPS", &maxFps_, 30.0f, 240.0f);
			if (oldMaxFps != maxFps_) {
				frameRateLimiter_->SetTargetFPS(maxFps_);
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

			if (isObjectValid()) {
				SceneObject& selectedObject = sceneObjects_[selectedObjectIndex_];

				ImGui::Text("Editing: %s", selectedObject.name.c_str());
				ImGui::SameLine();
				ImGui::Text(" -> ");
				ImGui::SameLine();
				if (ImGui::Button("Jump To")) {
					const SceneObject& selectedObject = sceneObjects_[selectedObjectIndex_];
					glm::vec3 localSize = selectedObject.mesh->boundingBoxMax_ - selectedObject.mesh->boundingBoxMin_;
					glm::vec3 worldSize = localSize * selectedObject.scale;
					glm::vec3 localCenter = (selectedObject.mesh->boundingBoxMin_ + selectedObject.mesh->boundingBoxMax_) / 2.0f;
					glm::vec3 worldCenter = selectedObject.position + (localCenter * selectedObject.scale);
					float longestSide = std::max(std::max(worldSize.x, worldSize.y), worldSize.z);
					float fovRadians = glm::radians(camera_->Zoom);
					float idealDistance = (longestSide * 0.5f) / tan(fovRadians * 0.5f);
					idealDistance *= 1.5f;
					const float maxFocusDistance = 50.0f;
					float finalDistance = std::min(idealDistance, maxFocusDistance);
					const float minFocusDistance = 2.0f;
					finalDistance = std::max(finalDistance, minFocusDistance);
					glm::vec3 direction = glm::normalize(glm::vec3(0.5f, 0.4f, 1.0f));
					glm::vec3 newCameraPos = worldCenter - direction * finalDistance;
					camera_->SetPositionAndTarget(newCameraPos, worldCenter);
				}
				ImGui::Text("Model Material");

				ImGui::ColorEdit3("Model Color", glm::value_ptr(selectedObject.color));
				ImGui::SliderFloat("Shininess", &selectedObject.shininess, 2.0f, 256.0f);
				ImGui::SliderFloat("Specular Strength", &selectedObject.specularStrength, 0.0f, 2.0f);
				ImGui::SliderFloat("Ambient Strength", &selectedObject.ambientStrength, 0.0f, 1.0f);

				if (ImGui::Button("Reset Material")) {
					SceneObject defaultMaterial;
					selectedObject.color = defaultMaterial.color;
					selectedObject.shininess = defaultMaterial.shininess;
					selectedObject.specularStrength = defaultMaterial.specularStrength;
					selectedObject.ambientStrength = defaultMaterial.ambientStrength;
				}

				ImGui::Separator();
				ImGui::Text("Model Transform");
				ImGui::DragFloat3("Position", glm::value_ptr(selectedObject.position), 0.1f);
				ImGui::SliderFloat3("Rotation", glm::value_ptr(selectedObject.rotation), -180.0f, 180.0f);
				ImGui::DragFloat3("Scale", glm::value_ptr(selectedObject.scale), 0.05f);

				if (ImGui::Button("Reset Transform")) {
					selectedObject.position = selectedObject.initialPosition;
					selectedObject.rotation = glm::vec3(0.0f);
					selectedObject.scale = glm::vec3(1.0f);
				}
			}
			else {
				ImGui::Text("No object selected.");
			}
		}
		ImGui::End();
	}

	bool Application::isObjectValid() const {
		return selectedObjectIndex_ != -1 && selectedObjectIndex_ < sceneObjects_.size();
	}

	void Application::QueueModelLoad(const std::string& path) {
		std::cout << "File dropped: " << path << std::endl;
		pathToLoad_ = path;
	}

	void Application::Run() {
		glEnable(GL_DEPTH_TEST);

		while (isRunning_) {
			float currentFrame = static_cast<float>(glfwGetTime());
			deltaTime_ = currentFrame - lastFrame_;
			lastFrame_ = currentFrame;

			UpdateWindowState();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			HandleInput();

			UIRender();

			Render();

			frameRateLimiter_->Sleep();

			window_->SwapBuffersAndPollEvents();
		}
	}
}
