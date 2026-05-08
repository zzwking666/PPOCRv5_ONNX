cmake_minimum_required (VERSION 3.10)

# 如果没有定义 CMAKE_TOOLCHAIN_FILE，则设置为 vcpkg 的工具链文件路径,一般情况下再CMakeUserPresets.json中会设置CMAKE_TOOLCHAIN_FILE环境变量指向vcpkg.cmake文件，这里做一个检查，如果没有定义则设置默认路径
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

#设置QT路径 
set(QT_PATH "C:/Development/tools/Qt/6.7.3/msvc2019_64")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${QT_PATH}")

#设置RWUL路径
list(APPEND CMAKE_PREFIX_PATH "D:/Development/RWUL")

project ("PPOCRv5_ONNX" LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 对于qt项目他俩默认开
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#查找第三方库
find_package(OpenCV CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)
find_package(ICU REQUIRED COMPONENTS uc i18n)
find_package(RWUL CONFIG REQUIRED COMPONENTS core oso hoecd rqwu imgcore imgqt imevo)

add_subdirectory ("PPOCRv5_ONNX")
