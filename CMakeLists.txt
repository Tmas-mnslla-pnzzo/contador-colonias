cmake_minimum_required(VERSION 3.5)
project(ContadorColonias VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(OpenCV_DIR "C:/msys64/mingw64/lib/cmake/opencv4")
# Le decimos a CMake exactamente dónde instalamos OpenCV recién

# Cambiamos Qt5 por Qt6
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(OpenCV REQUIRED)

# Añadir los archivos fuente
add_executable(ContadorColonias
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

# Vinculamos con Qt6
target_link_libraries(ContadorColonias PRIVATE Qt6::Widgets ${OpenCV_LIBS})