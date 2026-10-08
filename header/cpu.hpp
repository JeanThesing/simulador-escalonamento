#pragma once
#include <iostream>
#include "tarefa.hpp"

//NUM DE PROCESSADORES NA SIMULACAO DEVEM SER ENTRE (valores entre 1 e N)
class CPU
{
private:
    int id;
    bool emUso = 0;
    bool ligado = 1;
    int tempoDesligado = 0; // sera atualizado pelo tick do sistema (int)
public:
    CPU( int id );
    ~CPU();
    void ocupar();
    void liberar();
    void ligar();
    void desligar();
    void switchLigado();
    void incrementaTempoDesligado(int tick); // sera incrementado de acordo com o tick
    void executarTarefa(Tarefa* tarefa);
};

//fila de prontos