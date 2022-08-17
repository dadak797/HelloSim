#ifndef GLFWOCCVIEW_H
#define GLFWOCCVIEW_H
//#include <GL/glew.h>
// #include <GLFW/glfw3.h>
// #include <GLFW/glfw3native.h>

#include "V3d_View.hxx"
#include "Opengl_context.hxx"
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewController.hxx>

struct GLFWwindow;
class OccWindow;
struct ImGuiContext;


class GlfwOccView : public AIS_ViewController
{
public:
	GlfwOccView();
	~GlfwOccView();

	void run();
	void initWindow();
	bool initOCC();
	void cleanup();
	void mainloop();
	void drawFrame();
	void OnWindowResized(int width, int height);
	void OnMouseButton(int button, int action, int mods);
	void OnMouseMove(int thePosX, int thePosY);
	static void s_onMouseMove(GLFWwindow* window, double thePosX, double thePosY);
	static void s_onWindowResized(GLFWwindow* window, int width, int height);
	static void s_onMouseButton(GLFWwindow* window, int button, int action, int mods);

	void redraw();
	void rotateXY(double degrees);

	GLFWwindow* window;
	
	Handle_V3d_Viewer m_viewer;
	Handle_V3d_View m_view;
	Handle_OpenGl_Context m_GLcontext;
	Handle_AIS_InteractiveContext mContext;
	Handle(OccWindow) hWnd;

	ImGuiContext* myImGuiContext { nullptr };

	bool m_isInitialized = false;
	bool m_needsResize = false;
	int ButtonMouseX = 0;
	int ButtonMouseY = 0;

		float loc[3] { 0.0f, 0.0f, 0.0f };

		void MakeBox(const float* loc);
};

#endif // !GLFWOCCVIEW_H
