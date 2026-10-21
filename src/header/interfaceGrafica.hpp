// Raylib: cria a janela do programa, controla o desenho e gerencia a execução gráfica.
#include "raylib.h"

// rlImGui: conecta a Raylib ao Dear ImGui, permitindo desenhar os componentes da interface.
#include "rlImGui.h"

// Dear ImGui: biblioteca utilizada para criar janelas, botões, tabelas e outros elementos da interface.
#include "imgui.h"

// Inclui a classe Simulador para acessar os dados reais da simulação.
#include "simulador.hpp"

#include <string>
#include <cstdio>

// Retorna a cor utilizada para representar uma tarefa no gráfico de Gantt.
// O ID -1 representa uma CPU ociosa.
ImU32 corTarefa(int id);

// Converte o estado de uma tarefa para um texto exibido na interface.
const char *nomeEstado(EstadoTarefa estado);

// Desenha o gráfico de Gantt com o histórico de execução das CPUs.
void desenharGantt(const Simulador &simulador);

// Desenha a tabela com as informações atuais das tarefas.
void desenharTarefas(const Simulador &simulador);
