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
    cpu.cpp
    relogio.cpp
    tarefa.cpp
    escalonadorRm.cpp
    escalonadorEdf.cpp
    simulador.cpp
    leitorConfig.cpp
)

# =========================
# Simulador no terminal
# =========================

add_executable(main
    main.cpp
    ${CORE_SOURCES}
)

target_include_directories(main PRIVATE
    header
)

# =========================
# Interface gráfica
# =========================

add_executable(testeGrafico
    testeGrafico.cpp

    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp

    external/rlImGui/rlImGui.cpp
)

target_include_directories(testeGrafico PRIVATE
    header
    external/imgui
    external/rlImGui
)

target_link_libraries(testeGrafico PRIVATE
    raylib
    opengl32
    gdi32
    winmm
)