#pragma once
#include <iostream>
#include "tarefa.hpp"
#include "cpu.hpp"
#include "relogio.hpp"
#include "escalonador.hpp"
#include "escalonadorRm.hpp"
#include "escalonadorEdf.hpp"
#include <list>
#include <vector>
#include <string>

//NUM DE PROCESSADORES NA SIMULACAO DEVEM SER ENTRE (valores entre 1 e N)
class Simulador
{
private:
    int qtde_cpus = 1;
    int quantum;
    Escalonador escalonador;
    std::string algoritmo_escalonamento = nullptr;
    // algoritmo de escalonamento a ser usado
    Relogio relogio = Relogio();
    std::vector<Tarefa> tarefas;// todas as tarefas
    std::vector<CPU> cpus; // todas as cpus
    std::list<int> listaDeProntas = std::list<int>(); // lista com o ids das tarefas prontas

public:
    Simulador( int qtde_cpus, int quantum, std::string algoritmo_escalonamento);
    ~Simulador();
    void criaCPUS();
    void criaEscalonador();
    void criaTarefas();
};

//fila de prontos