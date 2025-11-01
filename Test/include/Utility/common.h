#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <game.h>
#include <scene.h>
#include <window.h>
#include <shader.hpp>
#include <system.h>
#include <filesystem.hpp>
#include <model.h>
#include <utility.h>
#include <spriteRenderer.h>

#include <iostream>
#include <vector>
#include <map>

#include <locale>
