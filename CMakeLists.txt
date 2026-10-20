# para rodar o build do cmake
# cmake -S . -B build -G Ninja
# cmake --build build
# ./build/main.exe
# ./build/testeGrafico.exe

cmake_minimum_required(VERSION 3.20)

project(simulador_escalonamento)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================
# Arquivos principais
# =========================

set(CORE_SOURCES
    src/cpu.cpp
    src/relogio.cpp
    src/tarefa.cpp
    src/escalonadorRm.cpp
    src/escalonadorEdf.cpp
    src/simulador.cpp
    src/leitorConfig.cpp
    src/escalonador.cpp
)

# =========================
# Simulador no terminal
# =========================

add_executable(main
    src/main.cpp
    ${CORE_SOURCES}
)

target_include_directories(main PRIVATE
    src/header
)

# =========================
# Interface gráfica
# =========================

add_executable(testeGrafico
    src/testeGrafico.cpp

    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp

    external/rlImGui/rlImGui.cpp
)

target_include_directories(testeGrafico PRIVATE
    src/header
    external/imgui
    external/rlImGui
)

target_link_libraries(testeGrafico PRIVATE
    raylib
    opengl32
    gdi32
    winmm
)