######################################################################
# Automatically generated by qmake (2.01a) Mon Jan 21 12:08:39 2008
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += $(HOME)/lcgtk/glslKernel/ .
LIBS += -L$(HOME)/lcgtk/glslKernel/ -lglslKernel -lglut

QT += opengl

# Input
HEADERS += application.h \
           camera.h \
           ellipse_rasterization.h \
           ewa_surface_splatting.h \
           file_io.h \
           kd-tree.h \
           materials.h \
           modelsTreeWidget.h \
           object.h \
           openGLWidget.h \
           ply.h \
           point_based_render.h \
           pprMainWindow.h \
           primitives.h \
           pyramid_point_render.h \
           pyramid_point_render_nearest.h \
           pyramid_point_render_color.h \
           pyramid_point_render_lod.h \
           pyramid_point_render_trees.h \
           pyramid_triangle_renderer.h \
           pyramid_types.h \
           quat.h \
           surfels.h \      
           timer.h \
           triangle_renderer.h \
           matrix.cc
FORMS += interface.ui
SOURCES += application.cc \
           camera.cc \
           ellipse_rasterization.cc \
           ewa_surface_splatting.cc \
           main.cpp \
           matrix.cc \
           modelsTreeWidget.cpp \
           object.cc \
           ply.c \
           point_based_render.cc \
           pprMainWindow.cpp \
           primitives.cc \
           pyramid_point_render.cc \
           pyramid_point_render_nearest.cc \
           pyramid_point_render_color.cc \
           pyramid_point_render_lod.cc \
           pyramid_point_render_trees.cc \
           pyramid_triangle_renderer.cc \
           surfels.cc \
           timer.c \
           triangle_renderer.cc
