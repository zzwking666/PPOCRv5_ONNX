set(DEMO_TARGET Utility)

# GLOB扫一层目录下的所有.cpp文件,GLOB_RECURSE扫多层目录下的所有.cpp文件
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# GLOB扫一层目录下的所有.h和.hpp文件,GLOB_RECURSE扫多层目录下的所有.h和.hpp文件
file(GLOB INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB INCLUDE_HPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
list(APPEND INCLUDE_FILES ${INCLUDE_HPP_FILES})

# 添加库生成目标
add_library(${DEMO_TARGET} ${SRC_FILES} ${INCLUDE_FILES})

# 添加头文件搜索路径
target_include_directories(${DEMO_TARGET} 
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接底层依赖库
target_link_libraries(${DEMO_TARGET}
	PUBLIC RWUL::oso
	PUBLIC Qt6::Core
)

add_library(Global::${DEMO_TARGET} ALIAS ${DEMO_TARGET})


