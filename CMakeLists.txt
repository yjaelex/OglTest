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

source_group( "GL" FILES
    GL/glcorearb.h
    GL/glext.h
	GL/glextensions.cpp
	GL/glextensions.h
	GL/glextensions.inl
    GL/wglext.h
)

# Function for building single example
function(buildTest TEST_NAME)
	message(STATUS "Adding test: ${TEST_NAME} \n")

	file(GLOB TEST_SRC "${TEST_NAME}/*.cpp")
	message(STATUS " ---> test main src: ${TEST_SRC} \n")
	file(GLOB TEST_HDR "${TEST_NAME}/*.h")
	message(STATUS " ---> test main hdr: ${TEST_HDR} \n")

	source_group( "Source" FILES
		${TEST_SRC}
		${TEST_HDR}
	)

	file(GLOB COMMON_SRC "common/*.cpp")
	file(GLOB COMMON_HDR "common/*.h")
	message(STATUS " ---> common src: ${COMMON_SRC} ${COMMON_HDR} \n")

	# Add shaders
	file(GLOB SHADERS_GLSL "${TEST_NAME}/*.vert" "${TEST_NAME}/*.frag" "${TEST_NAME}/*.comp" "${TEST_NAME}/*.geom" "${TEST_NAME}/*.tesc" "${TEST_NAME}/*.tese" "${TEST_NAME}/*.mesh" "${SHADER_DIR_GLSL}/*.task" "${TEST_NAME}/*.rgen" "${TEST_NAME}/*.rchit" "${TEST_NAME}/*.rmiss")

	source_group("Shaders\\GLSL" FILES ${SHADERS_GLSL})
	message(STATUS " ---> test shaders: ${SHADERS_GLSL}\n")

	if(WIN32)
	add_executable(${TEST_NAME} WIN32
		${TEST_SRC}
		${TEST_HDR}
		${COMMON_SRC}
		${COMMON_HDR}
		${SHADERS_GLSL}
		${SHADERS_HLSL}
	)
	target_link_libraries(${TEST_NAME} ${ALL_LIBS})

	else(WIN32)
	add_executable(${TEST_NAME} 
		${TEST_SRC}
		${TEST_HDR}
		${COMMON_MAIN}
		${COMMON_SHADER}
		${SHADERS_GLSL}
		${SHADERS_HLSL}
	)

	target_link_libraries( ${TEST_NAME} ${ALL_LIBS} GL EGL pthread dl )

	endif(WIN32)

	SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )

	add_custom_command(
		TARGET ${TEST_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${TEST_NAME}${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}/"
	)
endfunction(buildTest)

# Build all examples
function(buildAllTests)
	foreach(TEST ${TESTS})
		buildTest(${TEST})
	endforeach(TEST)
endfunction(buildAllTests)

set(TESTS
	test1_red_triangle
	test2_separate_program
	test3_more_separate
	test4_tess
	test5_tess_full
)

buildAllTests()
