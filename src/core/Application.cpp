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
		selectedEntity = 0;
		prevEscapeKeyState = GLFW_RELEASE;

		if (!glfwInit()) {
			throw std::runtime_error("Failed to initialize GLFW");
		}

		window = std::make_unique<Window>(800, 600, "Cerberus Project");
		camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
		shader = std::make_unique<Shader>("res/shaders/simple.vert", "res/shaders/simple.frag");
		normalsShader = std::make_unique<Shader>("res/shaders/normals.vert", "res/shaders/normals.frag");
		gizmo = std::make_unique<Gizmo>();
		pivotVisualizer = std::make_unique<PivotVisualizer>();
		grid = std::make_unique<Grid>();
		frameRateLimiter = std::make_unique<FrameRateLimiter>(maxFps);

		glfwSetInputMode(window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetDropCallback(window->GetNativeWindow(), drop_callback);
		scene.CreateEntity();
		LoadDefaultModel();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(window->GetNativeWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	Application::~Application() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();
	}

	void Application::ProcessInput(float deltaTime) {
		GLFWwindow* window = this->window->GetNativeWindow();

		int currentEscapeState = glfwGetKey(window, GLFW_KEY_ESCAPE);
		if (currentEscapeState == GLFW_RELEASE && prevEscapeKeyState == GLFW_PRESS) {
			controlMode = (controlMode == ControlMode::Camera) ? ControlMode::UI : ControlMode::Camera;
		}
		prevEscapeKeyState = currentEscapeState;

		if (controlMode == ControlMode::Camera) {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera->ProcessKeyboard(FORWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera->ProcessKeyboard(BACKWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera->ProcessKeyboard(LEFT, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera->ProcessKeyboard(RIGHT, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera->ProcessKeyboard(UP, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera->ProcessKeyboard(DOWN, deltaTime);
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

		CreateSceneObjectFromMesh(std::move(newMesh), path.substr(path.find_last_of("/\\") + 1), path);
	}

	void Application::LoadDefaultModel() {
		std::cout << "Loading default model from memory.." << std::endl;

		const char* data = Cerberus::DefaultAssets::CubeObj.c_str();
		size_t size = Cerberus::DefaultAssets::CubeObj.length();
		std::unique_ptr<Mesh> defaultMesh = ObjLoader::LoadModelFromMemory(data, size);
		CreateSceneObjectFromMesh(std::move(defaultMesh), "Default Cube", "Internal Memory");
	}

	void Application::UpdateWindowState() {
		if (!pathToLoad.empty()) {
			LoadModelFromFile(pathToLoad);
			pathToLoad.clear();
		}

		if (window->ShouldClose()) {
			isRunning = false;
		}
	}

	void Application::HandleInput() {
		static ImGuiIO& io = ImGui::GetIO();
		ProcessInput(deltaTime);

		bool isCameraActive = (controlMode == ControlMode::Camera) && !io.WantCaptureMouse;

		if (controlMode == ControlMode::Camera) {
			glfwSetInputMode(window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			double xpos, ypos;
			glfwGetCursorPos(window->GetNativeWindow(), &xpos, &ypos);
			if (firstMouse) {
				lastX = xpos; lastY = ypos; firstMouse = false;
			}
			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos;
			lastX = xpos; lastY = ypos;
			camera->ProcessMouseMovement(xoffset, yoffset);
		}
		else {
			glfwSetInputMode(window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstMouse = true;
		}
	}

	void Application::Render() {
		float aspectRatio = static_cast<float>(window->GetWidth()) / static_cast<float>(window->GetHeight());
		glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), aspectRatio, 0.1f, 1000.0f);
		glm::mat4 view = camera->GetViewMatrix();

		glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		grid->Draw(view, projection, camera->Position);

		glm::mat4 selectedModelMatrix;
		bool hasSelectedEntity = false;

		for (Entity i = 0; i < scene.GetEntityCount(); ++i) {
			auto& meshComp = scene.GetComponent<MeshComponent>(i);
			if (!meshComp.mesh) continue;
			auto& transform = scene.GetComponent<TransformComponent>(i);
			auto& material = scene.GetComponent<MaterialComponent>(i);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, transform.position);
			model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, transform.scale);

			if (i == selectedEntity) {
				if (enableCulling) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
				if (renderMode == 0) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				else if (renderMode == 1) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				else { glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); glPointSize(3.0f); }

				selectedModelMatrix = model;
				hasSelectedEntity = true;
			}
			else {
				glDisable(GL_CULL_FACE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			shader->Use();
			shader->SetVec3("u_objectColor", material.color);
			shader->SetFloat("u_shininess", material.shininess);
			shader->SetFloat("u_specularStrength", material.specularStrength);
			shader->SetFloat("u_ambientStrength", material.ambientStrength);

			shader->SetVec3("u_lightColor", 1.0f, 1.0f, 1.0f);
			shader->SetVec3("u_lightPos", camera->Position);
			shader->SetVec3("u_viewPos", camera->Position);
			shader->SetMat4("u_Model", model);
			shader->SetMat4("u_View", view);
			shader->SetMat4("u_Projection", projection);
			meshComp.mesh->Draw(*shader);
		}

		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (hasSelectedEntity) {
			auto& meshComp = scene.GetComponent<MeshComponent>(selectedEntity);
			if (meshComp.mesh) {
				if (showPivot) {
					pivotVisualizer->Draw(selectedModelMatrix, view, projection);
				}
				if (showFaceNormals) {
					normalsShader->Use();
					normalsShader->SetMat4("u_Model", selectedModelMatrix);
					normalsShader->SetMat4("u_View", view);
					normalsShader->SetMat4("u_Projection", projection);
					meshComp.mesh->DrawFaceNormals();
				}
				if (showVertexNormals) {
					normalsShader->Use();
					normalsShader->SetMat4("u_Model", selectedModelMatrix);
					normalsShader->SetMat4("u_View", view);
					normalsShader->SetMat4("u_Projection", projection);
					meshComp.mesh->DrawVertexNormals();
				}
			}
		}

		gizmo->Draw(view, window->GetWidth(), window->GetHeight());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Application::UIRender() {
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Model Info")) {
					showModelInfoWindow = true;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Quit", "ESC")) {
					isRunning = false;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showModelInfoWindow) {
			if (ImGui::Begin("Model Information", &showModelInfoWindow)) {

				if (selectedEntity > 0 && selectedEntity < scene.GetEntityCount()) {

					const auto& tag = scene.GetComponent<TagComponent>(selectedEntity);
					const auto& meshComp = scene.GetComponent<MeshComponent>(selectedEntity);

					if (meshComp.mesh) {
						ImGui::Text("File Path: %s", tag.filePath.c_str());
						ImGui::Separator();
						ImGui::Text("Vertex Count: %zu", meshComp.mesh->m_Vertices.size());
						ImGui::Text("Index Count: %zu", meshComp.mesh->m_Indices.size());
						ImGui::Text("Triangle Count: %zu", meshComp.mesh->m_Indices.size() / 3);
					}
					else {
						ImGui::Text("Selected entity has no mesh data.");
					}
				}
				else {
					ImGui::Text("No object selected.");
				}

				ImGui::Separator();
				ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera->Position.x, camera->Position.y, camera->Position.z);
			}
			ImGui::End();
		}

		if (ImGui::Begin("Cerberus")) {
			if (ImGui::BeginChild("Hierarchy", ImVec2(ImGui::GetContentRegionAvail().x, 200), true)) {
				if (scene.GetEntityCount() <= 1) {
					ImGui::Text("No objects in scene.");
				}
				else {
					for (Entity i = 1; i < scene.GetEntityCount(); ++i) {
						auto& tag = scene.GetComponent<TagComponent>(i);
						if (ImGui::Selectable(tag.name.c_str(), selectedEntity == i)) {
							selectedEntity = i;
							auto& meshComp = scene.GetComponent<MeshComponent>(i);
							if (meshComp.mesh) pivotVisualizer->UpdateSize(meshComp.mesh->boundingRadius_);
						}
					}
				}
			}
			ImGui::EndChild();

			if (ImGui::Button("Destroy") && selectedEntity > 0) {
				scene.DestroyEntity(selectedEntity);
				selectedEntity = 0;
			}

			ImGui::Separator();
			ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
			float oldMaxFps = maxFps;
			ImGui::SliderFloat("Max FPS", &maxFps, 30.0f, 240.0f);
			if (oldMaxFps != maxFps) {
				frameRateLimiter->SetTargetFPS(maxFps);
			}
			ImGui::Separator();
			ImGui::Text("Camera Settings");
			ImGui::SliderFloat("Movement Speed", &camera->MovementSpeed, 1.0f, 100.0f);
			if (camera->MovementSpeed > 100)
				camera->MovementSpeed = 100;
			ImGui::Text("Render Settings");
			ImGui::Separator();
			ImGui::RadioButton("Solid", &renderMode, 0); ImGui::SameLine();
			ImGui::RadioButton("Wireframe", &renderMode, 1); ImGui::SameLine();
			ImGui::RadioButton("Points", &renderMode, 2);
			ImGui::Text("View Settings");
			ImGui::Separator();
			ImGui::Checkbox("Enable Culling", &enableCulling);
			ImGui::Checkbox("Show Face Normals", &showFaceNormals);
			ImGui::SameLine();
			ImGui::Checkbox("Show Vertex Normals", &showVertexNormals);
			ImGui::Checkbox("Show Model Pivot", &showPivot);
			ImGui::Separator();

			if (selectedEntity > 0) {
				auto& tag = scene.GetComponent<TagComponent>(selectedEntity);
				auto& transform = scene.GetComponent<TransformComponent>(selectedEntity);
				auto& material = scene.GetComponent<MaterialComponent>(selectedEntity);
				auto& meshComp = scene.GetComponent<MeshComponent>(selectedEntity);

				ImGui::Text("Editing: %s", tag.name.c_str());
				ImGui::SameLine();
				ImGui::Text(" -> ");
				ImGui::SameLine();
				if (ImGui::Button("Jump To")) {
					const auto& selectedTransform = scene.GetComponent<TransformComponent>(selectedEntity);
					const auto& selectedMeshComp = scene.GetComponent<MeshComponent>(selectedEntity);
					glm::vec3 localSize = selectedMeshComp.mesh->boundingBoxMax_ - selectedMeshComp.mesh->boundingBoxMin_;
					glm::vec3 worldSize = localSize * selectedTransform.scale;
					glm::vec3 localCenter = (selectedMeshComp.mesh->boundingBoxMin_ + selectedMeshComp.mesh->boundingBoxMax_) / 2.0f;
					glm::vec3 worldCenter = selectedTransform.position + (localCenter * selectedTransform.scale);

					float longestSide = std::max({ worldSize.x, worldSize.y, worldSize.z });
					float fovRadians = glm::radians(camera->Zoom);
					float idealDistance = (longestSide * 0.5f) / tan(fovRadians * 0.5f);
					idealDistance *= 1.5f;

					const float maxFocusDistance = 50.0f;
					float finalDistance = std::min(idealDistance, maxFocusDistance);
					const float minFocusDistance = 2.0f;
					finalDistance = std::max(finalDistance, minFocusDistance);

					glm::vec3 direction = glm::normalize(glm::vec3(0.5f, 0.4f, 1.0f));
					glm::vec3 newCameraPos = worldCenter - direction * finalDistance;

					camera->SetPositionAndTarget(newCameraPos, worldCenter);
				}
				ImGui::Text("Model Material");

				ImGui::ColorEdit3("Model Color", glm::value_ptr(material.color));
				ImGui::SliderFloat("Shininess", &material.shininess, 2.0f, 256.0f);
				ImGui::SliderFloat("Specular Strength", &material.specularStrength, 0.0f, 2.0f);
				ImGui::SliderFloat("Ambient Strength", &material.ambientStrength, 0.0f, 1.0f);
				if (ImGui::Button("Reset Material")) {
					auto& selectedMaterial = scene.GetComponent<MaterialComponent>(selectedEntity);
					selectedMaterial = MaterialComponent{};
				}

				ImGui::Separator();
				ImGui::Text("Model Transform");
				ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.1f);
				ImGui::SliderFloat3("Rotation", glm::value_ptr(transform.rotation), -180.0f, 180.0f);
				ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.05f);
				if (ImGui::Button("Reset Transform")) {
					auto& selectedTransform = scene.GetComponent<TransformComponent>(selectedEntity);
					selectedTransform.position = selectedTransform.initialPosition;
					selectedTransform.rotation = glm::vec3(0.0f);
					selectedTransform.scale = glm::vec3(1.0f);
				}

			}
			else {
				ImGui::Text("No object selected.");
			}
		}
		ImGui::End();
	}

	void Application::CreateSceneObjectFromMesh(std::unique_ptr<Mesh> mesh, const std::string& name, const std::string& path) {
		if (!mesh) {
			std::cerr << "Failed to create scene object from an invalid mesh." << std::endl;
			return;
		}

		Entity newEntity = scene.CreateEntity(name);

		auto& tag = scene.GetComponent<TagComponent>(newEntity);
		tag.filePath = path;

		auto& transform = scene.GetComponent<TransformComponent>(newEntity);
		transform.position = glm::vec3(0.0f);
		transform.initialPosition = glm::vec3(0.0f);

		auto& meshComp = scene.GetComponent<MeshComponent>(newEntity);
		pivotVisualizer->UpdateSize(mesh->boundingRadius_);
		meshComp.mesh = std::move(mesh);

		selectedEntity = newEntity;
		std::cout << "Successfully added '" << name << "' to the scene." << std::endl;
	}

	void Application::QueueModelLoad(const std::string& path) {
		std::cout << "File dropped: " << path << std::endl;
		pathToLoad = path;
	}

	void Application::Run() {
		glEnable(GL_DEPTH_TEST);

		while (isRunning) {
			float currentFrame = static_cast<float>(glfwGetTime());
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			UpdateWindowState();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			HandleInput();

			UIRender();

			Render();

			frameRateLimiter->Sleep();

			window->SwapBuffersAndPollEvents();
		}
	}
}
