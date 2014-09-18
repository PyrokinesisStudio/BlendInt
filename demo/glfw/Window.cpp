#include "Window.hpp"

#include <iostream>

//using BlendInt::Interface;
#include <BlendInt/Gui/AbstractWidget.hpp>
#include <BlendInt/Window/KeyEvent.hpp>
#include <BlendInt/Window/MouseEvent.hpp>

namespace BlendInt {

	static KeyEvent global_key_event;
	static MouseEvent global_mouse_event;

	static void CbError (int error, const char* description)
	{
		std::cerr << "Error: " << description
				  << " (error code: " << error << ")"
				  << std::endl;
	}

	static void CbWindowSize(GLFWwindow* window, int w, int h)
	{
		Interface::instance->Resize(w, h);
	}

	static void CbKey(GLFWwindow* window, int key, int scancode, int action,
					  int mods)
	{
		switch (action) {
			case GLFW_PRESS:
                global_key_event.set_action(KeyPress);
				break;
			case GLFW_RELEASE:
                global_key_event.set_action(KeyRelease);
				break;
			case GLFW_REPEAT:
                global_key_event.set_action(KeyRepeat);
				break;
			default:
                global_key_event.set_action(KeyNone);
				break;
		}

		global_key_event.set_key(key);
		global_key_event.set_scancode(scancode);
		global_key_event.set_modifiers(mods);
        global_key_event.clear_text();

		Interface::instance->DispatchKeyEvent(global_key_event);
	}

	static void CbChar(GLFWwindow* window, unsigned int character)
	{

#ifdef __APPLE__
		// glfw3 in Mac OS will call this function if press some unprintalbe keys such as Left, Right, Up, Down
			if(character > 255) {
				DBG_PRINT_MSG("unprintable character in Mac: %u", character);
				return;
			}
#endif

		global_key_event.set_text(character);

		Interface::instance->DispatchKeyEvent(global_key_event);
	}

	static void CbMouseButton(GLFWwindow* window, int button, int action,
							  int mods)
	{
		MouseAction mouse_action = MouseNone;

		switch (action) {
			case GLFW_RELEASE:
				mouse_action = MouseRelease;
				break;
			case GLFW_PRESS:
				mouse_action = MousePress;
				break;
			case GLFW_REPEAT:
				mouse_action = MouseNone;
				break;
			default:
				break;
		}

		MouseButton mouse_button = MouseButtonNone;

		switch(button) {
			case GLFW_MOUSE_BUTTON_1:
				mouse_button = MouseButtonLeft;
				break;
			case GLFW_MOUSE_BUTTON_2:
				mouse_button = MouseButtonRight;
				break;
			case GLFW_MOUSE_BUTTON_3:
				mouse_button = MouseButtonMiddle;
				break;
			default:
				break;
		}

		global_mouse_event.set_button(mouse_button);
		global_mouse_event.set_action(mouse_action);
		global_mouse_event.set_modifiers(mods);

		Interface::instance->DispatchMouseEvent(global_mouse_event);
	}

	static void CbCursorPos(GLFWwindow* window, double xpos, double ypos)
	{
        global_mouse_event.set_action(MouseMove);
        global_mouse_event.set_button(MouseButtonNone);
		global_mouse_event.set_global_position(static_cast<int>(xpos), Interface::instance->GetCurrentContextHeight() - static_cast<int>(ypos));

		Interface::instance->DispatchMouseEvent(global_mouse_event);
	}

	void Init ()
	{
		/* Initialize the library */
		if (!glfwInit())
			exit(-1);

		glfwSetErrorCallback(&CbError);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        //glfwWindowHint(GLFW_REFRESH_RATE, 1);

	}

	GLFWwindow* CreateWindow (const char* name, int width, int height)
	{
		GLFWwindow* window = glfwCreateWindow(width, height, name, NULL, NULL);
		if (!window) {
			glfwTerminate();
			exit(-1);
		}

		glfwSetWindowSizeCallback(window, &CbWindowSize);
		glfwSetKeyCallback(window, &CbKey);
		glfwSetCharCallback(window, &CbChar);
		glfwSetMouseButtonCallback(window, &CbMouseButton);
		glfwSetCursorPosCallback(window, &CbCursorPos);

		/* Make the window's context current */
		glfwMakeContextCurrent(window);
	
		/* initialize BlendInt after OpenGL content is created */
		if (!Interface::Initialize()) {
			glfwTerminate();
			exit(-1);
		}

		Interface::instance->Resize(width, height);
	
		return window;
	}

	void RunLoop (GLFWwindow* window, RenderCallback callback, void* param)
	{
		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window)) {
			/* Render here */
			Interface::instance->Draw();

            if(callback) {
                (*callback)(param);
            }

			/* Swap front and back buffers */
			glfwSwapBuffers(window);
		
			/* Poll for and process events */
#ifdef __APPLE__
            glfwWaitEvents();
#else
            //glfwWaitEvents();
            glfwPollEvents();
#endif  // __APPLE__
		}
	
	}

	void Terminate ()
	{
		/* release BlendInt */
		Interface::Release();
	
		glfwTerminate();
	}

	bool CheckAllocatedObjects ()
	{
		return true;
	}

}
