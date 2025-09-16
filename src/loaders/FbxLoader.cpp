#include "FbxLoader.h"
#include <iostream>
#include <vector>
#include <fbxsdk.h>

namespace Cerberus {

    // Recursive function to process nodes in the FBX scene graph
    void ProcessNode(FbxNode* node, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
    void ProcessMesh(FbxMesh* mesh, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);

    std::unique_ptr<Mesh> FbxLoader::LoadModel(const std::string& path) {
        // 1. Initialize the FBX SDK Manager
        FbxManager* sdkManager = FbxManager::Create();
        FbxIOSettings* ioSettings = FbxIOSettings::Create(sdkManager, IOSROOT);
        sdkManager->SetIOSettings(ioSettings);

        // 2. Create an Importer
        FbxImporter* importer = FbxImporter::Create(sdkManager, "");
        if (!importer->Initialize(path.c_str(), -1, sdkManager->GetIOSettings())) {
            std::cerr << "FbxImporter initialization failed: " << importer->GetStatus().GetErrorString() << std::endl;
            sdkManager->Destroy();
            return nullptr;
        }

        // 3. Create a scene object and import the file
        FbxScene* scene = FbxScene::Create(sdkManager, "myScene");
        importer->Import(scene);
        importer->Destroy();

        // Ensure geometry is triangulated
        FbxGeometryConverter geoConverter(sdkManager);
        geoConverter.Triangulate(scene, true);

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // 4. Process the scene graph starting from the root node
        FbxNode* rootNode = scene->GetRootNode();
        if (rootNode) {
            for (int i = 0; i < rootNode->GetChildCount(); i++) {
                ProcessNode(rootNode->GetChild(i), vertices, indices);
            }
        }

        // 5. Clean up the SDK
        sdkManager->Destroy();

        if (vertices.empty()) {
            std::cerr << "No valid mesh data found in FBX file: " << path << std::endl;
            return nullptr;
        }

        std::cout << "Loaded FBX model: " << path << " with " << vertices.size() << " vertices." << std::endl;
        return std::make_unique<Mesh>(vertices, indices);
    }

    void ProcessNode(FbxNode* node, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
        // Process the mesh for this node, if it exists
        FbxMesh* mesh = node->GetMesh();
        if (mesh) {
            ProcessMesh(mesh, vertices, indices);
        }

        // Recursively process all child nodes
        for (int i = 0; i < node->GetChildCount(); i++) {
            ProcessNode(node->GetChild(i), vertices, indices);
        }
    }

    void ProcessMesh(FbxMesh* mesh, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
        FbxVector4* controlPoints = mesh->GetControlPoints();
        int polygonCount = mesh->GetPolygonCount();

        int firstVertexIndex = vertices.size();

        for (int i = 0; i < polygonCount; i++) {
            for (int j = 0; j < 3; j++) { // We assume triangles
                int controlPointIndex = mesh->GetPolygonVertex(i, j);

                Vertex vertex;
                // Position
                FbxVector4 pos = controlPoints[controlPointIndex];
                vertex.Position = glm::vec3(pos[0], pos[1], pos[2]);

                // Normal
                FbxVector4 normal;
                mesh->GetPolygonVertexNormal(i, j, normal);
                vertex.Normal = glm::vec3(normal[0], normal[1], normal[2]);

                vertices.push_back(vertex);
                indices.push_back(vertices.size() - 1);
            }
        }
    }
}