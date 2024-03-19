cmake_minimum_required(VERSION 3.16)

set(LIBARY_TARGET_NAME VTKViewer)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(libs REQUIRED)
find_package(VTK REQUIRED COMPONENTS
        vtkCommonCore
        vtkViewsQt
        vtkRenderingCore
        vtkRenderingImage
        vtkRenderingFreeType
        vtkRenderingOpenGL2
        vtkRenderingVolumeOpenGL2
        vtkInteractionImage
        vtkInteractionStyle
        vtkIOGeometry
)
include(${VTK_USE_FILE})
find_package(Qt5 REQUIRED COMPONENTS
        Core
        Widgets
        Gui
)

set(${LIBARY_TARGET_NAME}_SOURCES
        vtkResliceCursorCallback.cpp
        QuadQtVTKRenderWidget.cpp
        VTKOpenGLWidget.cpp
)

set(${LIBARY_TARGET_NAME}_HEADERS
        vtkResliceCursorCallback.h
        QuadQtVTKRenderWidget.h
        VTKOpenGLWidget.h
)

set(QT_LIBRARIES
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
)
set(OTHER_LIBRARIES
        ${VTK_LIBRARIES}
        ${QT_LIBRARIES}
)

add_library(${LIBARY_TARGET_NAME} STATIC
        ${${LIBARY_TARGET_NAME}_SOURCES}
        ${${LIBARY_TARGET_NAME}_HEADERS}
)

# target_include_directories(${LIBARY_TARGET_NAME} PUBLIC
#         ${VTK_INCLUDE_DIRS}
#         "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
# )

target_link_libraries(${LIBARY_TARGET_NAME} PUBLIC
        ${OTHER_LIBRARIES}
)
