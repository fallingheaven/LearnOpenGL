cmake_minimum_required(VERSION 3.29)
project(LearnOpenGL)

set(CMAKE_CXX_STANDARD 17)

# 把 glad.c 编译成一个静态库
add_library(glad STATIC src/glad.c)

file(GLOB IMGUI_SOURCES "lib/imgui/*.cpp" "lib/imgui/backend/*.cpp")

file(GLOB Utility "src/Utility/*.cpp")

# 通用配置函数
function(configure_target target_name sources)
    add_executable(${target_name}
            ${sources}
            "src/Utility/stb_implementation.cpp" # 用于保证单次实现
    )

    target_include_directories(${target_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/Utility
        ${CMAKE_SOURCE_DIR}/lib/imgui
        ${CMAKE_SOURCE_DIR}/lib/imgui/backend
        ${CMAKE_SOURCE_DIR}/lib/stb
#        ${CMAKE_SOURCE_DIR}/lib/freetype-2.14.1
#        ${CMAKE_SOURCE_DIR}/lib/freetype-2.14.1/include
        ${CMAKE_SOURCE_DIR}/thirdparty/glfw/include
        ${CMAKE_SOURCE_DIR}/lib/assimp/assimp-install/include/assimp
    )

    find_package(assimp REQUIRED)

    set(FREETYPE_DIR ${CMAKE_SOURCE_DIR}/lib/freetype-2.14.1)
    set(FREETYPE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lib/freetype-2.14.1/include)
    find_package(Freetype REQUIRED)

    target_link_libraries(${target_name} PRIVATE
        glad
        glfw3dll
        opengl32
        user32
        gdi32
        shell32
        assimp::assimp
        Freetype::Freetype
    )

    target_sources(${target_name} PRIVATE
        ${IMGUI_SOURCES}
        ${Utility}
    )

    # 运行时复制 DLL
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/thirdparty/glfw/lib-mingw-w64/glfw3.dll"
        $<TARGET_FILE_DIR:${target_name}>
    )
endfunction()

# 主程序
file(GLOB SOURCES "src/*.cpp")
configure_target(${PROJECT_NAME} "${SOURCES}")

set(PROJECT_ROOT "src/LearnOpenGL")

# 添加新的可执行文件
configure_target(window "${PROJECT_ROOT}/Basic/Window/window.cpp")

configure_target(triangle "${PROJECT_ROOT}/Basic/Triangle/triangle.cpp")

configure_target(light "${PROJECT_ROOT}/Light/light.cpp")

configure_target(model "${PROJECT_ROOT}/Model/model.cpp")

configure_target(advanced "${PROJECT_ROOT}/Advanced/advanced.cpp")

configure_target(advancedLighting "${PROJECT_ROOT}/AdvancedLighting/advancedLighting.cpp")

configure_target(pbr "${PROJECT_ROOT}/PBR/pbr.cpp")

configure_target(practice "${PROJECT_ROOT}/Practice/practice.cpp")