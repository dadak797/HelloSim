// Copyright (c) 2019 OPEN CASCADE SAS
//
// This file is part of the examples of the Open CASCADE Technology software library.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE

#include "GlfwOcctView.h"

#include <AIS_Shape.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <TopAbs_ShapeEnum.hxx>

#include <AIS_ViewCube.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Aspect_RenderingContext.hxx>
#include <OpenGl_Context.hxx>


#include <iostream>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <spdlog/spdlog.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <future>


namespace
{
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
}

GlfwOcctView::GlfwOcctView()
{
}

GlfwOcctView::~GlfwOcctView()
{
}

GlfwOcctView* GlfwOcctView::toView(GLFWwindow* theWin)
{
    return static_cast<GlfwOcctView*>(glfwGetWindowUserPointer(theWin));
}

void GlfwOcctView::errorCallback(int theError, const char* theDescription)
{
    Message::DefaultMessenger()->Send(TCollection_AsciiString("Error") + theError + ": " + theDescription, Message_Fail);
}

void GlfwOcctView::run()
{
    initWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);
    initViewer();
    initImGui();
    initDemoScene();
    if (myView.IsNull())
    {
        return;
    }

    myView->MustBeResized();
    myOcctWindow->Map();
    mainloop();
    
    cleanupImGui();
    cleanup();
}

void GlfwOcctView::initWindow(int theWidth, int theHeight, const char* theTitle)
{
    glfwSetErrorCallback(GlfwOcctView::errorCallback);
    glfwInit();

    const bool toAskCoreProfile = true;
    if (toAskCoreProfile)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    #if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    myOcctWindow = new GlfwOcctWindow(theWidth, theHeight, theTitle);
    glfwSetWindowUserPointer(myOcctWindow->getGlfwWindow(), this);

    // Window callback
    glfwSetWindowSizeCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onResizeCallback);
    glfwSetFramebufferSizeCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onFBResizeCallback);

    // Mouse callback
    glfwSetScrollCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onMouseScrollCallback);
    glfwSetMouseButtonCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onMouseButtonCallback);
    glfwSetCursorPosCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onMouseMoveCallback);
}

void GlfwOcctView::initViewer()
{
    if (myOcctWindow.IsNull() || myOcctWindow->getGlfwWindow() == nullptr)
    {
        return;
    }

    Handle(OpenGl_GraphicDriver) aGraphicDriver = new OpenGl_GraphicDriver(myOcctWindow->GetDisplay(), false);  // myOcctWindow->GetDisplay() is null
    Handle(V3d_Viewer) aViewer = new V3d_Viewer(aGraphicDriver);

    myTextStyle = new Prs3d_TextAspect();
    myTextStyle->SetFont(Font_NOF_ASCII_MONO);
    myTextStyle->SetHeight(12);
    myTextStyle->Aspect()->SetColor(Quantity_NOC_LIGHTGRAY);
    myTextStyle->Aspect()->SetColorSubTitle(Quantity_NOC_BLACK);
    myTextStyle->Aspect()->SetDisplayType(Aspect_TODT_SHADOW);
    myTextStyle->Aspect()->SetTextFontAspect(Font_FA_Bold);
    myTextStyle->Aspect()->SetTextZoomable(false);
    myTextStyle->SetHorizontalJustification(Graphic3d_HTA_LEFT);
    myTextStyle->SetVerticalJustification(Graphic3d_VTA_BOTTOM);

    aViewer->SetDefaultLights();
    aViewer->SetLightOn();
    aViewer->SetDefaultTypeOfView(V3d_PERSPECTIVE);
    aViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);

    myView = aViewer->CreateView();
    myView->SetImmediateUpdate(false);
    myView->SetWindow(myOcctWindow, myOcctWindow->NativeGlContext());
    myView->ChangeRenderingParams().ToShowStats = true;
    myView->SetBgGradientColors(Quantity_Color(107./ 255., 140./ 255., 222./ 255., Quantity_TOC_RGB), 
                                Quantity_Color(255./255., 255./255., 255./255., Quantity_TOC_RGB), Aspect_GFM_VER);
    SetContinuousRedraw(true);


    myContext = new AIS_InteractiveContext(aViewer);

    // For Solid
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_Dynamic)->SetColor(Quantity_NOC_LIGHTCYAN1);
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_Dynamic)->SetTransparency(0.5f);
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_Dynamic)->SetDisplayMode(AIS_Shaded);
    // For Vertex, Edge, Face
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_LocalDynamic)->SetColor(Quantity_NOC_LIGHTCYAN1);
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_LocalDynamic)->SetTransparency(0.2f);
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_LocalDynamic)->SetDisplayMode(AIS_Shaded);
    // For Selected Solid
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_Selected)->SetColor(Quantity_NOC_DEEPSKYBLUE1);
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_Selected)->SetTransparency(0.2f);
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_Selected)->SetDisplayMode(AIS_Shaded);
    // For Selected Vertex, Edge, Face
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_LocalSelected)->SetColor(Quantity_NOC_INDIANRED);
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_LocalSelected)->SetTransparency(0.2f);
    myContext->HighlightStyle(Prs3d_TypeOfHighlight_LocalSelected)->SetDisplayMode(AIS_Shaded);

    // To Remove Unnecessary Wires On Curved Surface
    myContext->SetIsoNumber(0);

    myContext->DefaultDrawer()->SetFaceBoundaryDraw(Standard_True);
    myContext->SetDisplayMode(AIS_Shaded, Standard_False);

    myGLContext = aGraphicDriver->GetSharedContext();
}

void GlfwOcctView::initImGui()
{
    // ImGUI Initialization
    myImGuiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(myImGuiContext);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();
    io.Fonts->AddFontFromFileTTF("resource/fonts/Consolas.ttf", 13.0f);

    ImGui_ImplGlfw_InitForOpenGL(myOcctWindow->getGlfwWindow(), false);
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    ImGui_ImplOpenGL3_CreateDeviceObjects();
}

void GlfwOcctView::cleanupImGui()
{
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(myImGuiContext);
}

void GlfwOcctView::initDemoScene()
{
    if (myContext.IsNull())
    {
        return;
    }

    //myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_WIREFRAME);

    // View cube is used instead of TriedronDisplay
    myViewCube = new AIS_ViewCube();
    // presentation parameters
    initPixelScaleRatio();
    myViewCube->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_TriedronPers, Aspect_TOTP_RIGHT_LOWER, Graphic3d_Vec2i(150, 150)));
    myViewCube->Attributes()->SetDatumAspect(new Prs3d_DatumAspect());
    myViewCube->Attributes()->DatumAspect()->SetTextAspect(myTextStyle);
    // animation parameters
    myViewCube->SetViewAnimation(myViewAnimation);
    myViewCube->SetFixedAnimationLoop(true);
    myViewCube->SetAutoStartAnimation(true);
    myContext->Display(myViewCube, false);

    // Make a box and a cone for demo scene
    gp_Ax2 anAxis;
    anAxis.SetLocation(gp_Pnt(0.0, 0.0, 0.0));
    Handle(AIS_Shape) aBox = new AIS_Shape(BRepPrimAPI_MakeBox(anAxis, 50, 50, 50).Shape());
    aBox->SetColor(Quantity_Color(Quantity_NOC_LIGHTGRAY));
    myContext->Display(aBox, AIS_Shaded, 0, false);
    anAxis.SetLocation(gp_Pnt(25.0, 125.0, 0.0));
    Handle(AIS_Shape) aCone = new AIS_Shape(BRepPrimAPI_MakeCone(anAxis, 25, 0, 50).Shape());
    aCone->SetColor(Quantity_Color(Quantity_NOC_LIGHTGRAY));
    myContext->Display(aCone, AIS_Shaded, 0, false);

    TCollection_AsciiString aGlInfo;
    {
        TColStd_IndexedDataMapOfStringString aRendInfo;
        myView->DiagnosticInformation (aRendInfo, Graphic3d_DiagnosticInfo_Basic);
        for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter(aRendInfo); aValueIter.More(); aValueIter.Next())
        {
            if (!aGlInfo.IsEmpty()) { aGlInfo += "\n"; }
            aGlInfo += TCollection_AsciiString("  ") + aValueIter.Key() + ": " + aValueIter.Value();
        }
    }
    Message::DefaultMessenger()->Send(TCollection_AsciiString("OpenGL info:\n") + aGlInfo, Message_Info);
}

void GlfwOcctView::initPixelScaleRatio()
{
    SetTouchToleranceScale(myDevicePixelRatio);
    if (!myView.IsNull())
    {
        myView->ChangeRenderingParams().Resolution = (unsigned int)(96.0 * myDevicePixelRatio + 0.5);
    }
    if (!myContext.IsNull())
    {
        myContext->SetPixelTolerance(int(myDevicePixelRatio * 6.0));
        if (!myViewCube.IsNull())
        {
            static const double THE_CUBE_SIZE = 60.0;
            myViewCube->SetSize(myDevicePixelRatio * THE_CUBE_SIZE, false);
            myViewCube->SetBoxFacetExtension(myViewCube->Size() * 0.15);
            myViewCube->SetAxesPadding(myViewCube->Size() * 0.10);
            myViewCube->SetFontHeight(THE_CUBE_SIZE * 0.16);
            if (myViewCube->HasInteractiveContext())
            {
                myContext->Redisplay(myViewCube, false);
            }
        }
    }
}

void GlfwOcctView::mainloop()
{
    while (!glfwWindowShouldClose(myOcctWindow->getGlfwWindow()))
    {
        // glfwPollEvents() for continuous rendering (immediate return if there are no new events)
        // and glfwWaitEvents() for rendering on demand (something actually happened in the viewer)
        //glfwPollEvents();
        glfwWaitEvents();

        if (!myView.IsNull())
        {
            FlushViewEvents(myContext, myView, true);
            
        //   myGLContext->MakeCurrent();

        //   ImGui_ImplGlfw_NewFrame();
        //   ImGui_ImplOpenGL3_NewFrame();
        //   ImGui::NewFrame();


        //   if (ImGui::Begin("my first ImGui window")) {
        //       ImGui::Text("This is first text...");
        //   }
        //   ImGui::End();

        //   ImGui::Render();
        //   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //   myGLContext->SwapBuffers();
        }
    }
}

void GlfwOcctView::cleanup()
{
    if (!myView.IsNull())
    {
        myView->Remove();
    }
    if (!myOcctWindow.IsNull())
    {
        myOcctWindow->Close();
    }
    glfwTerminate();
}

void GlfwOcctView::onResize(int theWidth, int theHeight)
{
    if (theWidth != 0 && theHeight != 0 && !myView.IsNull())
    {
        myView->Window()->DoResize();
        myView->MustBeResized();
        myView->Invalidate();
        myView->Redraw();
    }
}

void GlfwOcctView::onMouseScroll(double theOffsetX, double theOffsetY)
{
    if (!myView.IsNull())
    {
        UpdateZoom(Aspect_ScrollDelta(myOcctWindow->CursorPosition(), int(theOffsetY * 8.0)));
    }
}

void GlfwOcctView::onMouseButton(int theButton, int theAction, int theMods)
{
    if (myView.IsNull()) { return; }

    const Graphic3d_Vec2i aPos = myOcctWindow->CursorPosition();
    if (theAction == GLFW_PRESS)
    {
        PressMouseButton(aPos, mouseButtonFromGlfw(theButton), keyFlagsFromGlfw(theMods), false);
    }
    else
    {
        ReleaseMouseButton(aPos, mouseButtonFromGlfw(theButton), keyFlagsFromGlfw(theMods), false);
    }
}

void GlfwOcctView::onMouseMove(int thePosX, int thePosY)
{
    const Graphic3d_Vec2i aNewPos(thePosX, thePosY);
    if (!myView.IsNull())
    {
        UpdateMousePosition(aNewPos, PressedMouseButtons(), LastMouseFlags(), false);
    }
}