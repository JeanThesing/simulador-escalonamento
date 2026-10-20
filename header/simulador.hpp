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
#include <memory>
#include "historico.hpp"

/*No entanto, com objetivo de terminar a simulação, assume-se que uma
tarefa periódica termina após ela ter executado 10 vezes.*/
//somente tarefas periodicas

// NUM DE PROCESSADORES NA SIMULACAO DEVEM SER ENTRE (valores entre 1 e N)
class Simulador
{
private:
    std::vector<EstadoSistema> historico; // historico do sistema
    int qtde_cpus = 1;
    int quantum;
    std::unique_ptr<Escalonador> escalonador;
    std::string algoritmo_escalonamento = "";
    // algoritmo de escalonamento a ser usado
    Relogio relogio = Relogio();
    std::vector<Tarefa> tarefas;                       // todas as tarefas
    std::vector<CPU> cpus;                             // todas as cpus
    std::vector<int> filaDeProntas = std::vector<int>(); // fila com o ids das tarefas prontas (vector para poder ordenar as tarefas de acordo com o algoritmo de escalonamento)

public:
    Simulador(int qtde_cpus, int quantum, std::string algoritmo_escalonamento);
    ~Simulador();

    void criaCPUS();
    void criaEscalonador();
    void criaTarefas();

    void avancaTick();
    void retrocedeTick(); // funcionara com base no historico do sistema (a ser implementado)
    void atualizaFilaDeProntas(); // atualiza a fila de tarefas prontas de acordo com o algoritmo de escalonamento
    void executarSimulacao();
    void salvaHistoricoTick();
};

// fila de prontos