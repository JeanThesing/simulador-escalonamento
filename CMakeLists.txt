# para rodar o build do cmake
# cmake -S . -B build -G Ninja
# cmake --build build
# testes de build (muda o diretorio)
# ./build/main.exe
# .\build\main.exe "tests\config.txt"
# .\build\main.exe "C:\teste-config\config.txt"  
#cmake --build build

# excluir pasta build
#Remove-Item -Recurse -Force build

# ./build/simulador.exe

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

add_executable(simulador
    src/interfaceGrafica.cpp
    src/simulador.cpp
    src/tarefa.cpp
    src/cpu.cpp
    src/relogio.cpp
    src/escalonador.cpp
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
