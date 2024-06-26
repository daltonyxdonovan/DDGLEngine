#!/bin/bash

g++ -o DDGL \
    Source.cpp \
    Renderer.cpp \
    VertexBuffer.cpp \
    IndexBuffer.cpp \
    VertexBufferLayout.cpp \
    VertexArray.cpp \
    Shader.cpp \
    src/vendor/stb_image/stb_image.cpp \
    src/Texture.cpp \
    imgui_draw.cpp \
    imgui.cpp \
    imgui_tables.cpp \
    imgui_widgets.cpp \
    imgui_impl_glfw.cpp \
    CubeCollider.cpp \
    imgui_impl_opengl3.cpp \
    imgui_stdlib.cpp \
    -lGLEW -lglfw -lGL -lX11 -lpthread -lXrandr -lsfml-audio -lsfml-system
