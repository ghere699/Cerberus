#include "Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

void framebuffer_size_callback(GLFWwindow* glfwWindow, int width, int height)
{
    auto window = static_cast<Cerberus::Window*>(glfwGetWindowUserPointer(glfwWindow));
    if (window) {
        window->OnResize(width, height);
    }
}

namespace Cerberus {

    Window::Window(int width, int height, const std::string& title)
        : m_Width(width), m_Height(height), m_Title(title) {

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
        if (!m_Window) {
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwSetWindowUserPointer(m_Window, this);

        glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

        glfwMakeContextCurrent(m_Window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        OnResize(width, height);
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
    }

    void Window::OnResize(int width, int height) {
        if (width == 0 || height == 0) return;

        m_Width = width;
        m_Height = height;
        glViewport(0, 0, m_Width, m_Height); 
    }

    bool Window::ShouldClose() const {
        return glfwWindowShouldClose(m_Window);
    }

    void Window::SwapBuffersAndPollEvents() {
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}