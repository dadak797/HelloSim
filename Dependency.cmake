# ExternalProject 관련 명령어 셋 추가
include(ExternalProject)

# Dependency 관련 변수 설정
set(DEP_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)  # PROJECT_BINARY_DIR = build
set(DEP_INCLUDE_DIR ${DEP_INSTALL_DIR}/include)
set(DEP_LIB_DIR ${DEP_INSTALL_DIR}/lib)

# spdlog: fast logger library
ExternalProject_Add(
    dep_spdlog
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG "v1.x"
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
    TEST_COMMAND ""
)

# Dependency 리스트 및 라이브러리 파일 리스트 추가
set(DEP_LIST ${DEP_LIST} dep_spdlog)
set(DEP_LIBS ${DEP_LIBS} spdlog$<$<CONFIG:Debug>:d>)

if(EMSCRIPTEN)
    message(STATUS "Emscripten used")
    set_target_properties(${PROJECT_NAME} 
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS "-O3 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=1 --shell-file ${CMAKE_SOURCE_DIR}/shell_minimal.html --preload-file ${CMAKE_SOURCE_DIR}@/"
    )
else()
    message(STATUS "Emscripten not used")
    # glfw
    ExternalProject_Add(
        dep_glfw
        GIT_REPOSITORY "https://github.com/glfw/glfw.git"
        GIT_TAG "3.3.3"
        GIT_SHALLOW 1
        UPDATE_COMMAND "" PATCH_COMMAND "" TEST_COMMAND ""
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
            -DGLFW_BUILD_EXAMPLES=OFF
            -DGLFW_BUILD_TESTS=OFF
            -DGLFW_BUILD_DOCS=OFF
    )

    # Dependency 리스트 및 라이브러리 파일 리스트 추가
    set(DEP_LIST ${DEP_LIST} dep_glfw)
    set(DEP_LIBS ${DEP_LIBS} glfw3)

    # glad
    ExternalProject_Add(
        dep_glad
        GIT_REPOSITORY "https://github.com/Dav1dde/glad.git"
        GIT_TAG "v0.1.36"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
            -DGLAD_INSTALL=ON
        TEST_COMMAND ""
    )

    set(DEP_LIST ${DEP_LIST} dep_glad)
    set(DEP_LIBS ${DEP_LIBS} glad)
endif()

# ImGUI
set(IMGUI_VERSION "1.88")
add_library(imgui
    dependencies/ImGui-${IMGUI_VERSION}/imgui_draw.cpp
    dependencies/ImGui-${IMGUI_VERSION}/imgui_tables.cpp
    dependencies/ImGui-${IMGUI_VERSION}/imgui_widgets.cpp
    dependencies/ImGui-${IMGUI_VERSION}/imgui.cpp
    dependencies/ImGui-${IMGUI_VERSION}/imgui_impl_glfw.cpp
    dependencies/ImGui-${IMGUI_VERSION}/imgui_impl_opengl3.cpp
    dependencies/ImGui-${IMGUI_VERSION}/imgui_demo.cpp
)
target_include_directories(imgui PRIVATE ${DEP_INCLUDE_DIR})
add_dependencies(imgui ${DEP_LIST})
set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ImGui-${IMGUI_VERSION})
set(DEP_LIST ${DEP_LIST} imgui)
set(DEP_LIBS ${DEP_LIBS} imgui)

# OCCT
set(OCCT_VERSION "7.6.0")
set(OpenCASCADE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dependencies/OCCT-win-${OCCT_VERSION}/inc)
set(OpenCASCADE_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/dependencies/OCCT-win-${OCCT_VERSION}/lib)
set(OpenCASCADE_LIBS
    TKBin TKBinL TKBinTObj TKBinXCAF TKBO TKBool TKBRep TKCAF TKCDF TKD3DHost 
    TKD3DHostTest TKDCAF TKDFBrowser TKDraw TKernel TKFeat TKFillet TKG2d TKG3d TKGeomAlgo
    TKGeomBase TKHLR TKIGES TKIVtk TKIVtkDraw TKLCAF TKMath TKMesh TKMeshVS TKMessageModel
    TKMessageView TKOffset TKOpenGl TKOpenGles TKOpenGlesTest TKOpenGlTest TKPrim TKQADraw TKRWMesh TKService 
    TKShapeView TKShHealing TKStd TKStdL TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase TKSTL TKTInspector
    TKTInspectorAPI TKTObj TKTObjDRAW TKToolsDraw TKTopAlgo TKTopTest TKTreeModel TKV3d TKVCAF TKView 
    TKViewerTest TKVInspector TKVRML TKXCAF TKXDEDRAW TKXDEIGES TKXDESTEP TKXMesh TKXml TKXmlL 
    TKXmlTObj TKXmlXCAF TKXSBase TKXSDRAW
)
set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${OpenCASCADE_INCLUDE_DIR})
set(DEP_LIB_DIR ${DEP_LIB_DIR} ${OpenCASCADE_LIBRARY_DIR})
set(DEP_LIBS ${DEP_LIBS} ${OpenCASCADE_LIBS})

# OpenVR
set(OpenVR_VERSION "1.14.15")
set(OpenVR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dependencies/OpenVR-win-${OpenVR_VERSION}/inc)
set(OpenVR_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/dependencies/OpenVR-win-${OpenVR_VERSION}/lib)
set(OpenVR_LIBS
    openvr_api
)
set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${OpenVR_INCLUDE_DIR})
set(DEP_LIB_DIR ${DEP_LIB_DIR} ${OpenVR_LIBRARY_DIR})
set(DEP_LIBS ${DEP_LIBS} ${OpenVR_LIBS})

# FreeImage
set(FreeImage_VERSION "3.17.0")
set(FreeImage_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dependencies/FreeImage-win-${FreeImage_VERSION}/inc)
set(FreeImage_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/dependencies/FreeImage-win-${FreeImage_VERSION}/lib)
set(FreeImage_LIBS
    FreeImage FreeImagePlus
)
set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${FreeImage_INCLUDE_DIR})
set(DEP_LIB_DIR ${DEP_LIB_DIR} ${FreeImage_LIBRARY_DIR})
set(DEP_LIBS ${DEP_LIBS} ${FreeImage_LIBS})

# FreeType
set(FreeType_VERSION "2.5.5")
set(FreeType_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dependencies/FreeType-win-${FreeType_VERSION}/inc)
set(FreeType_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/dependencies/FreeType-win-${FreeType_VERSION}/lib)
set(FreeType_LIBS
    freetype
)
set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${FreeType_INCLUDE_DIR})
set(DEP_LIB_DIR ${DEP_LIB_DIR} ${FreeType_LIBRARY_DIR})
set(DEP_LIBS ${DEP_LIBS} ${FreeType_LIBS})

# ffmpeg
set(ffmpeg_VERSION "3.3.4")
set(ffmpeg_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dependencies/ffmpeg-win-${ffmpeg_VERSION}/inc)
set(ffmpeg_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/dependencies/ffmpeg-win-${ffmpeg_VERSION}/lib)
set(ffmpeg_LIBS
    avcodec avdevice avfilter avformat avutil swscale
)
set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${ffmpeg_INCLUDE_DIR})
set(DEP_LIB_DIR ${DEP_LIB_DIR} ${ffmpeg_LIBRARY_DIR})
set(DEP_LIBS ${DEP_LIBS} ${ffmpeg_LIBS})

# tbb
set(tbb_VERSION "2017.0.100")
set(tbb_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dependencies/tbb-win-${tbb_VERSION}/inc/tbb)
set(tbb_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/dependencies/tbb-win-${tbb_VERSION}/lib/intel64/vc_mt)
set(tbb_LIBS
    tbb tbb_debug tbb_preview tbb_preview_debug tbbmalloc tbbmalloc_debug tbbmalloc_proxy tbbmalloc_proxy_debug tbbproxy tbbproxy_debug
)
set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${tbb_INCLUDE_DIR})
set(DEP_LIB_DIR ${DEP_LIB_DIR} ${tbb_LIBRARY_DIR})
set(DEP_LIBS ${DEP_LIBS} ${tbb_LIBS})