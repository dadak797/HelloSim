// #include <GLFW/glfw3.h>
// #if defined (__APPLE__)
//     #define GLFW_EXPOSE_NATIVE_COCOA
//     #define GLFW_EXPOSE_NATIVE_NSGL
// #elif defined (_WIN32)
//     #define GLFW_EXPOSE_NATIVE_WIN32
//     #define GLFW_EXPOSE_NATIVE_WGL
// #else
//     #define GLFW_EXPOSE_NATIVE_X11
//     #define GLFW_EXPOSE_NATIVE_GLX
// #endif
// #include <GLFW/glfw3native.h>
#include "GlfwOccWindow.h"

#if defined (__APPLE__)
    #undef Handle // avoid name collisions in macOS headers
    #define GLFW_EXPOSE_NATIVE_COCOA
    #define GLFW_EXPOSE_NATIVE_NSGL
#elif defined (_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define GLFW_EXPOSE_NATIVE_WGL
#else
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_GLX
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>


IMPLEMENT_STANDARD_RTTIEXT(OccWindow, Aspect_Window)

OccWindow::OccWindow(GLFWwindow* window)
{
    if(window==NULL)
        return;
    mWindow=window;
    int xpos, ypos;
    glfwGetWindowPos(mWindow, &xpos, &ypos);
    int width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    myXLeft   = xpos;
    myYTop    = ypos;
    myXRight  = xpos+width;
    myYBottom = ypos+height;
}

void OccWindow::Destroy()
{
    mWindow = NULL;
}

Aspect_Drawable OccWindow::NativeParentHandle() const
{
    return 0;
}

Aspect_RenderingContext OccWindow::NativeGlContext() const
{
#if defined (__APPLE__)
    return (NSOpenGLContext*)glfwGetNSGLContext(mWindow);
#elif defined (_WIN32)
    return glfwGetWGLContext(mWindow);
#else
    return glfwGetGLXContext(mWindow);
#endif
}

Aspect_Drawable OccWindow::NativeHandle() const
{
#if defined (__APPLE__)
  return (Aspect_Drawable)glfwGetCocoaWindow(mWindow);
#elif defined (_WIN32)
  return (Aspect_Drawable)glfwGetWin32Window(mWindow);
#else 
  return (Aspect_Drawable)glfwGetX11Window(mWindow);
#endif
}

Standard_Boolean OccWindow::IsMapped() const
{ 
    return glfwGetWindowAttrib(mWindow, GLFW_VISIBLE);
}

void OccWindow::Map() const
{
    glfwShowWindow(mWindow);
}

void OccWindow::Unmap() const
{
    glfwHideWindow(mWindow);
}

Aspect_TypeOfResize OccWindow::DoResize()
{
    int aMask = 0;
    Aspect_TypeOfResize aMode = Aspect_TOR_UNKNOWN;

    if (glfwGetWindowAttrib(mWindow, GLFW_VISIBLE) == 1)
    {
        int xpos, ypos;
        glfwGetWindowPos(mWindow, &xpos, &ypos);
        int width, height;
        glfwGetWindowSize(mWindow, &width, &height);
        if (Abs(xpos - myXLeft) > 2) aMask |= 1;
        if (Abs(xpos+width - myXRight) > 2) aMask |= 2;
        if (Abs(ypos - myYTop ) > 2) aMask |= 4;
        if (Abs(ypos+height - myYBottom) > 2) aMask |= 8;

        switch (aMask)
        {
        case 0:
            aMode = Aspect_TOR_NO_BORDER;
            break;
        case 1:
            aMode = Aspect_TOR_LEFT_BORDER;
            break;
        case 2:
            aMode = Aspect_TOR_RIGHT_BORDER;
            break;
        case 4:
            aMode = Aspect_TOR_TOP_BORDER;
            break;
        case 5:
            aMode = Aspect_TOR_LEFT_AND_TOP_BORDER;
            break;
        case 6:
            aMode = Aspect_TOR_TOP_AND_RIGHT_BORDER;
            break;
        case 8:
            aMode = Aspect_TOR_BOTTOM_BORDER;
            break;
        case 9:
            aMode = Aspect_TOR_BOTTOM_AND_LEFT_BORDER;
            break;
        case 10:
            aMode = Aspect_TOR_RIGHT_AND_BOTTOM_BORDER;
            break;
        default:
            break;
        }  // end switch

        *((Standard_Integer*)&myXLeft) = xpos;
        *((Standard_Integer*)&myXRight) = xpos + width;
        *((Standard_Integer*)&myYTop) = ypos;
        *((Standard_Integer*)&myYBottom) = ypos + height;
    }

    return aMode;
}

Standard_Real OccWindow::Ratio() const
{
    int width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    return Standard_Real(width) / Standard_Real(height);
}

void OccWindow::Size(Standard_Integer& theWidth, Standard_Integer& theHeight) const
{
    int width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    theWidth  = width;
    theHeight = height;
}

void OccWindow::Position(Standard_Integer& theX1, Standard_Integer& theY1,
                         Standard_Integer& theX2, Standard_Integer& theY2) const
{
    int xpos, ypos;
    glfwGetWindowPos(mWindow, &xpos, &ypos);
    int width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    theX1 = xpos;
    theX2 = xpos + width;
    theY1 = ypos;
    theY2 = ypos + height;
}

Graphic3d_Vec2i OccWindow::CursorPosition() const
{
    Graphic3d_Vec2d aPos;
    glfwGetCursorPos(mWindow, &aPos.x(), &aPos.y());
    return Graphic3d_Vec2i((int)aPos.x(), (int)aPos.y());
}