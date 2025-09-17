#pragma once

#include <string>
#include <glm.hpp>

namespace Cerberus {

    class Shader {
    public:
        Shader(const std::string& vertexPath, const std::string& fragmentPath);
        ~Shader();

        void Use() const;

        void SetMat4(const std::string& name, const glm::mat4& mat) const;
        void SetVec3(const std::string& name, const glm::vec3& value) const;
        void SetVec3(const std::string& name, float v1, float v2, float v3) const;
        void SetInt(const std::string& name, int value) const;
        void SetFloat(const std::string& name, float value) const;

    private:
        unsigned int m_ID; 

        void CheckCompileErrors(unsigned int shader, const std::string& type);
    };

}