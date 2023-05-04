cmake_minimum_required(VERSION 3.13)

set(BIN_NAME deep-learning-debugger)
project(${BIN_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(PkgConfig REQUIRED)

#for qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QtModule Core Gui Widgets Multimedia Charts)
find_package(Qt5 REQUIRED ${QtModule})

#for ncnn
pkg_check_modules(InferenceEngine REQUIRED IMPORTED_TARGET ncnn)
find_package(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

#for opencv
find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENCV REQUIRED IMPORTED_TARGET opencv4)

#for sources
include_directories(.)
include_directories(./src)

aux_source_directory(./src allSource)
aux_source_directory(./src/alg allSource)
aux_source_directory(./src/alg/detect allSource)
aux_source_directory(./src/ui allSource)
aux_source_directory(./src/ui/subpage allSource)
aux_source_directory(./src/ui/subwidget allSource)
aux_source_directory(./src/utils allSource)

add_executable(${BIN_NAME} ${allSource})

target_include_directories(${BIN_NAME} PUBLIC
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Multimedia Qt5::Charts
    ${PROJECT_INCLUDE}
    ${InferenceEngine_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Multimedia Qt5::Charts
    ${TARGET_LIBS}
    ${InferenceEngine_LIBRARIES}
    PkgConfig::OPENCV
)
