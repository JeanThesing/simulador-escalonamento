
# para rodar o build do cmake
# cmake -S . -B build -G Ninja
# cmake --build build

# testes de build (muda o diretorio)
# .\build\simulador.exe
# .\build\simulador.exe "tests\config.txt"
# .\build\simulador.exe "C:\teste-config\config.txt"

# recompilar
# cmake --build build

# excluir pasta build
# Remove-Item -Recurse -Force build

cmake_minimum_required(VERSION 3.20)

project(simulador_escalonamento)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================
# Raylib estática
# =========================

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/raylib)

# =========================
# Arquivos principais
# =========================

set(CORE_SOURCES
    src/cpu.cpp
    src/relogio.cpp
    src/tarefa.cpp
    src/simulador.cpp
    src/leitorConfig.cpp
    src/escalonadorPrioridade.cpp
)

# =========================
# Simulador principal
# =========================

add_executable(simulador
    src/main.cpp
    ${CORE_SOURCES}
    src/interfaceGrafica.cpp
    src/header/interfaceGrafica.hpp
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/rlImGui/rlImGui.cpp
)

target_include_directories(simulador PRIVATE
    src/header
    external/imgui
    external/rlImGui
)

# =========================
# Interface gráfica
# =========================

target_link_libraries(simulador PRIVATE
    raylib
)

if(MINGW)
    target_link_options(simulador PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
endif()
