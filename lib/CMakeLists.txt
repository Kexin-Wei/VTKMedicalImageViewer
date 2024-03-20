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

find_package(ITK REQUIRED COMPONENTS 
        ITKCommon
        ITKIOImageBase
        ITKIOJPEG
        ITKIOGDCM
        ITKImageIntensity
        ITKVTK
        ITKVtkGlue
        ITKIONRRD
        ITKIONIFTI
        ITKIOMINC
        ITKIOMeta
        ITKIOGDCM
)
INCLUDE(${ITK_USE_FILE})

set(${LIBARY_TARGET_NAME}_SOURCES
        BaseViewer.cpp
        SliceViewer.cpp
        Data3d.cpp
        VolumeImage.cpp
        Coordinate.cpp
        CoordinateRepository.cpp
        BaseData.cpp
        VtkCustomizedButton.cpp
        BaseOverlayButton.cpp
        QOverlayButton.cpp
        ViewerManager.cpp
        QuadViewerWidget.cpp
        StereoViewer.cpp
)

set(${LIBARY_TARGET_NAME}_HEADERS
        BaseViewer.h
        SliceViewer.h
        VisualizationDataType.h
        Data3d.h
        VolumeImage.h
        Coordinate.h
        CoordinateRepository.h
        BaseData.h      
        unit.h
        VtkCustomizedButton.h
        BaseOverlayButton.h
        QOverlayButton.h
        ViewerManager.h
        QuadViewerWidget.h  
        Constants.h    
        StereoViewer.h
)

set(QT_LIBRARIES
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
)
set(OTHER_LIBRARIES
        ${VTK_LIBRARIES}
        ${QT_LIBRARIES}
        ${ITK_LIBRARIES}
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
