if(MSVC AND NOT "${MSVC_VERSION}" LESS 1400)
    add_definitions( "/MP" )
endif()

if(LINUX_WINSYS_EGL)
set(GLEW_DEF
    -DGLEW_EGL
)
endif(LINUX_WINSYS_EGL)

add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
    -DGLEW_STATIC
    ${GLEW_DEF}
    -D_CRT_SECURE_NO_WARNINGS
)

#add_subdirectory (glfw-3.1.2)

include_directories(
#	glfw-3.1.2/include/GLFW/
    ./include/
    glew-2.2.0/include/
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(OPENGL_LIBRARY
    ${OPENGL_LIBRARY}
    GL GLU rt
    ${LINUX_WINSYS_LIB}
    ${CMAKE_DL_LIBS}
#	${GLFW_LIBRARIES}
)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

### GLEW ###

set(GLEW_SOURCE
    glew-2.2.0/src/glew.c
)

set(GLEW_HEADERS
)

add_library( GLEW_220 STATIC
    ${GLEW_SOURCE}
    ${GLEW_INCLUDE}
)

target_link_libraries(GLEW_220
    ${OPENGL_LIBRARY}
    ${EXTRA_LIBS}
)


