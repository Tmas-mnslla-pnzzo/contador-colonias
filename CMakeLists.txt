cmake_minimum_required(VERSION 3.5)
project(ContadorColonias VERSION 1.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(WIN32 AND NOT OpenCV_DIR AND EXISTS "C:/msys64/mingw64/lib/cmake/opencv4")
    set(OpenCV_DIR "C:/msys64/mingw64/lib/cmake/opencv4")
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

set(FUENTES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    calibracion.cpp
    calibracion.h
    recursos.qrc
)

if(WIN32)
    list(APPEND FUENTES icono.rc)
endif()

add_executable(ContadorColonias WIN32 ${FUENTES})

target_compile_definitions(ContadorColonias PRIVATE APP_VERSION="${PROJECT_VERSION}")

target_link_libraries(ContadorColonias PRIVATE Qt6::Widgets Qt6::Network ${OpenCV_LIBS})

install(TARGETS ContadorColonias
    RUNTIME DESTINATION bin
)
