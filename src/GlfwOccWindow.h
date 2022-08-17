#ifndef OCCWINDOW_H
#define OCCWINDOW_H

//#include <GLFW/glfw3.h>
//#include <GLFW/glfw3native.h>
#include <Aspect_Window.hxx>
#include <Aspect_RenderingContext.hxx>

struct GLFWwindow;


class OccWindow : public Aspect_Window
{
public:
    OccWindow(GLFWwindow* window);
    void Destroy();
    ~OccWindow()
    {
        Destroy();
    }

    // Return native OpenGL context.
    Aspect_RenderingContext NativeGlContext() const;

    //! Returns native Window handle
    Aspect_Drawable NativeHandle() const;
    
    //! Returns parent of native Window handle.
    Aspect_Drawable NativeParentHandle() const;
    
    //! Applies the resizing to the window <me>
    Aspect_TypeOfResize DoResize();
    
    //! Returns True if the window <me> is opened
    //! and False if the window is closed.
    Standard_Boolean IsMapped() const;
    
    //! Apply the mapping change to the window <me>
    //! and returns TRUE if the window is mapped at screen.
    Standard_Boolean DoMapping() const { return Standard_True; }
    
    //! Opens the window <me>.
    void Map() const;
    
    //! Closes the window <me>.
    void Unmap() const;
    void Position(Standard_Integer& theX1, Standard_Integer& theY1,
                  Standard_Integer& theX2, Standard_Integer& theY2) const;
    
    //! Returns The Window RATIO equal to the physical
    //! WIDTH/HEIGHT dimensions.
    Standard_Real Ratio() const;
    void Size(Standard_Integer& theWidth, Standard_Integer& theHeight) const;
    Aspect_FBConfig NativeFBConfig() const Standard_OVERRIDE { return NULL; }
    DEFINE_STANDARD_RTTIEXT(OccWindow, Aspect_Window)

    GLFWwindow* getGlfwWindow() { return mWindow; }

    Graphic3d_Vec2i CursorPosition() const;

//protected:
    Standard_Integer myXLeft;
    Standard_Integer myYTop;
    Standard_Integer myXRight;
    Standard_Integer myYBottom;
    GLFWwindow* mWindow;
};

#endif // OCCWINDOW_H