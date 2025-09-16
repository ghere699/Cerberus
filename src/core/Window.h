#pragma once

#include <string>

// Forward declare GLFWwindow to avoid including glfw3.h in the header
struct GLFWwindow;

namespace Cerberus {

    class Window {
    public:
        Window(int width, int height, const std::string& title);
        ~Window();

        // prevent copying
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool ShouldClose() const;
        void SwapBuffersAndPollEvents();
        GLFWwindow* GetNativeWindow() const { return m_Window; }

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        void OnResize(int width, int height);
    private:
        GLFWwindow* m_Window;
        int m_Width;
        int m_Height;
        std::string m_Title;
    };

}