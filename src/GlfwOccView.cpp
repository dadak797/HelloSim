//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <GLFW/glfw3native.h>
// #if defined (__APPLE__)
// 	#define GLFW_EXPOSE_NATIVE_COCOA
// 	#define GLFW_EXPOSE_NATIVE_NSGL
// #elif defined (_WIN32)
// 	#include <windows.h>
// 	#define GLFW_EXPOSE_NATIVE_WIN32
// 	#define GLFW_EXPOSE_NATIVE_WGL
// #else 
// 	#define GLFW_EXPOSE_NATIVE_X11
// 	#define GLFW_EXPOSE_NATIVE_GLX
// #endif
//#include "GLFW/glfw3native.h"
#include "GlfwOccView.h"
#include "GlfwOccWindow.h"
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_Handle.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <OpenGl_GraphicDriver.hxx>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <gp_Trsf.hxx>


static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

GlfwOccView::GlfwOccView(){}
GlfwOccView::~GlfwOccView() {}

void GlfwOccView::run()
{
	initWindow();
	mainloop();
	cleanup();
}

void GlfwOccView::initWindow()
{
	glfwSetErrorCallback(error_callback);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(800, 600, "occt", NULL, NULL);
	glfwSetWindowUserPointer(window, this);

	// window callback
	glfwSetWindowSizeCallback(window, GlfwOccView::s_onWindowResized);
	// mouse callback
	glfwSetMouseButtonCallback(window, GlfwOccView::s_onMouseButton);
	glfwSetCursorPosCallback(window, GlfwOccView::s_onMouseMove);

	//glfwMakeContextCurrent(window);
	//glewInit();

    initOCC();

		//std::cout << "before ImGui init" << std::endl;
    //imgui init
		// ImGUI Initialization
    myImGuiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(myImGuiContext);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();
    io.Fonts->AddFontFromFileTTF("resource/fonts/Consolas.ttf", 13.0f);

		//std::cout << "before ImGui_ImplGlfw_InitForOpenGL" << std::endl;

    ImGui_ImplGlfw_InitForOpenGL(hWnd->getGlfwWindow(), false);

		//std::cout << "before ImGui_ImplOpenGL3_Init" << std::endl;
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    ImGui_ImplOpenGL3_CreateDeviceObjects();

		//std::cout << "Init OCC done" << std::endl;
}


bool GlfwOccView::initOCC()
{
	if (!m_isInitialized) {
		Handle_Graphic3d_GraphicDriver gpxDriver;
		Handle(OpenGl_GraphicDriver) myGraphicDriver;
		Handle_Aspect_DisplayConnection dispConnection;
		if (dispConnection.IsNull())
			dispConnection = new Aspect_DisplayConnection;
		myGraphicDriver = new OpenGl_GraphicDriver(dispConnection,false);
		myGraphicDriver->SetBuffersNoSwap(true);
		// Create the named OCC 3d viewer
		m_viewer = new V3d_Viewer(myGraphicDriver);
		// Configure the OCC 3d viewer
		m_viewer->SetDefaultViewSize(800.);
		m_viewer->SetDefaultLights();
		m_viewer->SetLightOn();
		m_viewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);

		//m_GLcontext = new OpenGl_Context();
#if defined (__APPLE__)
		if (m_GLcontext->Init((NSOpenGLContext*)glfwGetNSGLContext(window)))
			fprintf(stderr, "opengl_context init success\n");
		m_view = m_viewer->CreateView();
		Handle(OccWindow) hWnd = new OccWindow(window);
			fprintf(stderr, "cocoa_window init success\n");
		m_view->SetWindow(hWnd, (NSOpenGLContext*)glfwGetNSGLContext(window));
#elif defined (_WIN32)
	    // if(m_GLcontext->Init(glfwGetWin32Window(window), GetDC(glfwGetWin32Window(window)), glfwGetWGLContext(window)))
			// fprintf(stderr, "opengl_context init success\n");
		m_view = m_viewer->CreateView();
		/*Handle(OccWindow)*/ hWnd = new OccWindow(window);
			// fprintf(stderr, "wnt_window init success %d %d %d %d\n", 
			// 	hWnd->myXLeft,hWnd->myXRight, hWnd->myYTop,hWnd->myYBottom);
		m_view->SetWindow(hWnd, hWnd->NativeGlContext());
		//std::cout << "m_GLcontext" << std::endl;
#else 
        if(m_GLcontext->Init(glfwGetX11Window(window), glfwGetX11Display(), glfwGetGLXContext(window)))
			fprintf(stderr, "opengl_context init success\n");
		m_view = m_viewer->CreateView();
		/*Handle(OccWindow)*/ hWnd = new OccWindow(window);
			fprintf(stderr, "x11_window init success %d %d %d %d\n", 
				hWnd->myXLeft,hWnd->myXRight, hWnd->myYTop,hWnd->myYBottom);
		m_view->SetWindow(hWnd,glfwGetGLXContext(window));
#endif

		m_GLcontext = myGraphicDriver->GetSharedContext();

		if (!hWnd->IsMapped())
			hWnd->Map();
		m_view->MustBeResized();
		//std::cout << "m_view resized" << std::endl;
		m_isInitialized = true;
		m_needsResize = true;
		mContext = new AIS_InteractiveContext(m_viewer);
		mContext->SetDisplayMode(AIS_Shaded, true);
		Handle(AIS_Shape) box = new AIS_Shape(BRepPrimAPI_MakeBox(50,50,50).Shape());
		m_view->FitAll();
		mContext->Display(box, Standard_False);

	}
	return true;
}


void GlfwOccView::MakeBox(const float* loc)
{
		Handle(AIS_Shape) box = new AIS_Shape(BRepPrimAPI_MakeBox(50,50,50).Shape());
		gp_Trsf trsf;
		gp_Vec trans(loc[0], loc[1], loc[2]);
		trsf.SetTranslation(trans);
		box->SetLocalTransformation(trsf);
		mContext->Display(box, Standard_False);
}

void GlfwOccView::cleanup()
{
    // todo cleanup imgui
		ImGui_ImplOpenGL3_DestroyFontsTexture();
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(myImGuiContext);


	//cleanup occt
	m_view->Remove();
	// cleanup glfw
	glfwDestroyWindow(window);
	glfwTerminate();
}


void GlfwOccView::s_onWindowResized(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0) return;
	GlfwOccView* app = reinterpret_cast<GlfwOccView*>(glfwGetWindowUserPointer(window));
	app->OnWindowResized(width,height);
}

void GlfwOccView::s_onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	  GlfwOccView* app = reinterpret_cast<GlfwOccView*>(glfwGetWindowUserPointer(window));
	  app->OnMouseButton(button, action, mods);
}

void GlfwOccView::s_onMouseMove(GLFWwindow* window, double thePosX, double thePosY)
{
		ImGui_ImplGlfw_CursorPosCallback(window, thePosX, thePosY);
	  GlfwOccView* app = reinterpret_cast<GlfwOccView*>(glfwGetWindowUserPointer(window));
	  app->OnMouseMove((int)thePosX, (int)thePosY);
}

void GlfwOccView::rotateXY(double degrees)
{
	if (m_view.IsNull())
		return;
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	int x = display_w / 2, y = display_h / 2;
	double X, Y, Z;
	m_view->Convert(x, y, X, Y, Z);
	m_view->Rotate(0, 0, degrees * M_PI / 180., X, Y, Z);
}

    // Convert GLFW mouse button into Aspect_VKeyMouse
    static Aspect_VKeyMouse mouseButtonFromGlfw(int theButton)
    {
        switch (theButton)
        {
            case GLFW_MOUSE_BUTTON_LEFT:   return Aspect_VKeyMouse_LeftButton;
            //case GLFW_MOUSE_BUTTON_RIGHT:  return Aspect_VKeyMouse_RightButton;
            //case GLFW_MOUSE_BUTTON_MIDDLE: return Aspect_VKeyMouse_MiddleButton;
            case GLFW_MOUSE_BUTTON_RIGHT:  return Aspect_VKeyMouse_MiddleButton;  // Right click to move
            case GLFW_MOUSE_BUTTON_MIDDLE: return Aspect_VKeyMouse_RightButton;  // Middle click to scale
        }
        return Aspect_VKeyMouse_NONE;
    }

    // Convert GLFW key modifiers into Aspect_VKeyFlags
    static Aspect_VKeyFlags keyFlagsFromGlfw(int theFlags)
    {
        Aspect_VKeyFlags aFlags = Aspect_VKeyFlags_NONE;
        if ((theFlags & GLFW_MOD_SHIFT) != 0)
        {
            aFlags |= Aspect_VKeyFlags_SHIFT;
        }
        if ((theFlags & GLFW_MOD_CONTROL) != 0)
        {
            aFlags |= Aspect_VKeyFlags_CTRL;
        }
        if ((theFlags & GLFW_MOD_ALT) != 0)
        {
            aFlags |= Aspect_VKeyFlags_ALT;
        }
        if ((theFlags & GLFW_MOD_SUPER) != 0)
        {
            aFlags |= Aspect_VKeyFlags_META;
        }
        return aFlags;
    }

void GlfwOccView::OnMouseButton(int button, int action, int mods)
{
	// double xpos, ypos;
	// glfwGetCursorPos(window, &xpos, &ypos);
	// ButtonMouseX = (int)xpos;
	// ButtonMouseY = (int)ypos;
	// if (button == GLFW_MOUSE_BUTTON_RIGHT)
	// {
	// 	if (action == GLFW_PRESS)
	// 	{
	// 		rotateXY(2.5);
	// 	}
	// }

		if (m_view.IsNull()) { return; }

    const Graphic3d_Vec2i aPos = hWnd->CursorPosition();
    if (action == GLFW_PRESS)
    {
        PressMouseButton(aPos, mouseButtonFromGlfw(button), keyFlagsFromGlfw(mods), false);
    }
    else
    {
        ReleaseMouseButton(aPos, mouseButtonFromGlfw(button), keyFlagsFromGlfw(mods), false);
    }
}

void GlfwOccView::OnMouseMove(int thePosX, int thePosY)
{
    const Graphic3d_Vec2i aNewPos(thePosX, thePosY);
    if (!m_view.IsNull())
    {
        UpdateMousePosition(aNewPos, PressedMouseButtons(), LastMouseFlags(), false);
    }
}

void GlfwOccView::mainloop()
{
	while (!glfwWindowShouldClose(window)) {
			drawFrame();
			glfwPollEvents();
	}
	

}

void GlfwOccView::OnWindowResized(int width, int height)
{
	if (!m_view.IsNull())
	{
		m_view->Window()->DoResize();
	}
	m_needsResize = true;
}



void GlfwOccView::redraw()
{
	if (!m_view.IsNull())
	{
		m_GLcontext->MakeCurrent();
		if (m_needsResize)
			m_view->MustBeResized();
		else
		{
			m_view->Invalidate();
		}
	}
	m_needsResize = false;
}


void GlfwOccView::drawFrame()
{
	if (!m_view.IsNull() )
	{
		m_GLcontext->MakeCurrent();

		if (m_needsResize)
			m_view->MustBeResized();
		else
		{
			m_view->Invalidate();
		}

		//std::cout << "imgui newframe" << std::endl;

		//todo imgui rendering
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();


			if (ImGui::Begin("my first ImGui window")) {
					ImGui::Text("This is first text...");
					if(ImGui::Button("Make Box")) {
						  MakeBox(loc);
					}
					ImGui::DragFloat3("Box location", loc);
			}
			ImGui::End();

			//std::cout << "imgui::end" << std::endl;
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		m_GLcontext->SwapBuffers();

		//std::cout << "swap buffer" << std::endl;

		FlushViewEvents(mContext, m_view, true);

		//std::cout << "FlushViewEvents" << std::endl;
	}
}
