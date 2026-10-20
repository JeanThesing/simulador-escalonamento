#pragma once
#include <iostream>
#include <vector>
#include "tarefa.hpp"

//fotografia do estado do simulador em determinado tick

struct EstadoCPU { // salva dados da CPU sem precisar se preocupar com ponteiros
    int id;
    int idTarefaAtual = -1; // id da tarefa atual que esta executando na CPU, -1 se nao tiver tarefa

    bool ligado;
    bool emUso;

    int tempoDesligado = 0;
};

/*exemplo:
EstadoCPU cpuSalva;

cpuSalva.id = 0;
cpuSalva.idTarefaAtual = 5;
cpuSalva.ligado = true;
cpuSalva.emUso = true;
cpuSalva.tempoDesligado = 0;
*/

struct EstadoSistema //salva o estado do sistema em determinado tick, para permitir retroceder o tempo da simulacao
{
    int tick = 0;

    std::vector<Tarefa> tarefas;
    std::vector<EstadoCPU> cpus;

    std::vector<int> filaDeProntas;

    int quantum = 0;
};