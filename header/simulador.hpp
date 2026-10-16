#pragma once
#include <iostream>
#include "tarefa.hpp"
#include "cpu.hpp"
#include "relogio.hpp"
#include "escalonador.hpp"
#include "escalonadorRm.hpp"
#include "escalonadorEdf.hpp"
#include <queue>
#include <vector>
#include <string>

struct EstadoSistema
{ // historico do sistema que vai ser adicionado a cada tick, para permitir retroceder o tempo da simulacao
    int tick;
    std::vector<Tarefa> tarefas;
    std::vector<CPU> cpus;
std::queue<int> filaDeProntas;
    int quantum;
};

// NUM DE PROCESSADORES NA SIMULACAO DEVEM SER ENTRE (valores entre 1 e N)
class Simulador
{
private:
    std::vector<EstadoSistema> historico = std::vector<EstadoSistema>(); //historico do sistema
    int qtde_cpus = 1;
    int quantum;
    Escalonador escalonador;
    std::string algoritmo_escalonamento = nullptr;
    // algoritmo de escalonamento a ser usado
    Relogio relogio = Relogio();
    std::vector<Tarefa> tarefas;                      // todas as tarefas
    std::vector<CPU> cpus;                            // todas as cpus
    std::queue<int> filaDeProntas = std::queue<int>(); // fila com o ids das tarefas prontas

public:
    Simulador(int qtde_cpus, int quantum, std::string algoritmo_escalonamento);
    ~Simulador();
    void criaCPUS();
    void criaEscalonador();
    void criaTarefas();
    void avancaTick();
    void retrocedeTick(); // funcionara com base no historico do sistema (a ser implementado)
    void executarSimulacao();
    void salvaHistoricoTick();
};

// fila de prontos