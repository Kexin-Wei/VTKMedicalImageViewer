cmake_minimum_required(VERSION 3.16)

set(libs_DIR "D:/UltrastRepos/libs/")
set(Qt5_DIR "C:/Qt/Qt5.12.10/5.12.10/msvc2015_64/lib/cmake/Qt5")
message(STATUS "Using MSVC")

project(TestVTKViewer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)

set(CMAKE_DEBUG_POSTFIX "d")

IF (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")
    add_compile_options(/W4)
ENDIF ()


add_subdirectory(lib)

find_package(Qt5 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
)

add_executable(${PROJECT_NAME}
        main.cpp
)


target_link_libraries(${PROJECT_NAME}
        VTKViewer
)

# copy dlls
# get windeployqt.exe path
# GET_TARGET_PROPERTY(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
# GET_FILENAME_COMPONENT(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
# FIND_PROGRAM(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
        --verbose 0
        --dir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>" $<TARGET_FILE:${PROJECT_NAME}>
)