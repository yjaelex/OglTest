cmake_minimum_required (VERSION 3.0)
project (OglTest)

find_package(OpenGL REQUIRED)

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Build dir is the same with source dir! Please select another Build Directory!" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces." )
endif()

if( MSVC )
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
elseif( UNIX)
    if( NOT CMAKE_BUILD_TYPE )
      set( CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release" FORCE )
    endif()

    set( CMAKE_CXX_FLAGS "-std=c++11 -g ${CMAKE_CXX_FLAGS} -DPOSIX=1 -DLINUX=1 -DNDEBUG=1" )
    set( CMAKE_CXX_FLAGS_DEBUG "-O0 -D_DEBUG=1" )
    set( CMAKE_CXX_FLAGS_RELEASE "-O3" )

    set( CMAKE_EXE_LINKER_FLAGS "-g" )
    set( CMAKE_SHARED_LINKER_FLAGS "-g" )
    set( CMAKE_MODULE_LINKER_FLAGS "-g" )
endif( MSVC )

file( GLOB_RECURSE HDRS *.h )

# Compile external dependencies 
add_subdirectory (external)

include_directories(
#	external/glfw-3.1.2/include/
	external/glm-0.9.9.8/
	external/glew-2.2.0/include/
#	external/assimp-3.0.1270/include/
	external/include/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
#	glfw
	GLEW_220
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# Test 1
source_group( "GL" FILES
    GL/glcorearb.h
    GL/glext.h
	GL/glextensions.cpp
	GL/glextensions.h
	GL/glextensions.inl
    GL/wglext.h
)

source_group( "Source" FILES
    test1_red_triangle/triangle.cpp
)

if(WIN32)
add_executable(test1_red_triangle WIN32
	test1_red_triangle/triangle.cpp
	common/main.cpp
                common/main.h
	common/shader.cpp
	common/shader.hpp

	test1_red_triangle/SimpleFragmentShader.fragmentshader
	test1_red_triangle/SimpleVertexShader.vertexshader
)
target_link_libraries(test1_red_triangle ${ALL_LIBS})

else(WIN32)
add_executable(test1_red_triangle
	test1_red_triangle/triangle.cpp
	common/main.cpp
                common/main.h
	common/shader.cpp
	common/shader.hpp
	
	test1_red_triangle/SimpleFragmentShader.fragmentshader
	test1_red_triangle/SimpleVertexShader.vertexshader
)

target_link_libraries( test1_red_triangle GL pthread dl )

endif(WIN32)


SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )

add_custom_command(
   TARGET test1_red_triangle POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/test1_red_triangle${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/test1_red_triangle/"
)

source_group( "Source" FILES
	test2_separate_program/separate_program.cpp
)

if(WIN32)
add_executable(test2_separate_program WIN32
	test2_separate_program/separate_program.cpp
	common/main.cpp
                common/main.h
	common/shader.cpp
	common/shader.hpp

	test2_separate_program/SepFragmentShader.frag
	test2_separate_program/SepVertexShader.vert
)
target_link_libraries(test2_separate_program ${ALL_LIBS})

else(WIN32)
add_executable(test2_separate_program 
	test2_separate_program/separate_program.cpp
	common/main.cpp
                common/main.h
	common/shader.cpp
	common/shader.hpp
	
	test2_separate_program/SepFragmentShader.frag
	test2_separate_program/SepVertexShader.vert
)

target_link_libraries( test2_separate_program GL pthread dl )

endif(WIN32)

add_custom_command(
   TARGET test2_separate_program POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/test2_separate_program${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/test2_separate_program/"
)
