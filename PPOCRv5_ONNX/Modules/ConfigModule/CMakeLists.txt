set(DEMO_TARGET ConfigModule)

# GLOB扫一层目录下的所有.cpp文件,GLOB_RECURSE扫多层目录下的所有.cpp文件
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# GLOB扫一层目录下的所有.h和.hpp文件,GLOB_RECURSE扫多层目录下的所有.h和.hpp文件
file(GLOB INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB INCLUDE_HPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
list(APPEND INCLUDE_FILES ${INCLUDE_HPP_FILES})

# GLOB扫一层目录下的所有.oso文件,GLOB_RECURSE扫多层目录下的所有.oso文件
file(GLOB OSO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/osoFile/*.oso")
oso_wrap_oso(GENERATED_HEADERS ${OSO_FILES} OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/osoInclude)

# 添加库生成目标
add_library(${DEMO_TARGET} ${SRC_FILES} ${INCLUDE_FILES})

# 添加头文件搜索路径
target_include_directories(${DEMO_TARGET} 
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/osoInclude
)

# 链接底层依赖库
target_link_libraries(${DEMO_TARGET}
	PUBLIC RWUL::oso
	PUBLIC Modules::IModule
	PUBLIC Global::Utility
)

add_library(Modules::${DEMO_TARGET} ALIAS ${DEMO_TARGET})


