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
// É a cor definida para a tarefa no arquivo de configuração (campo "cor").
ImU32 corTarefa(const Tarefa &tarefa);

// Converte o estado de uma tarefa para um texto exibido na interface.
const char *nomeEstado(EstadoTarefa estado);

// Desenha o gráfico de Gantt com o histórico de execução das CPUs.
void desenharGantt(const Simulador &simulador);

// Desenha a tabela com as informações atuais das tarefas.
void desenharTarefas(const Simulador &simulador);

// Desenha a legenda dos elementos mostrados no gráfico de Gantt da janela.
void desenharLegenda(const Simulador &simulador);
