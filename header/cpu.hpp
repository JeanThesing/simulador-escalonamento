#pragma once
#include <iostream>
#include "tarefa.hpp"

// NUM DE PROCESSADORES NA SIMULACAO DEVEM SER ENTRE (valores entre 1 e N)
class CPU
{
private:
    int id;
    bool emUso = 0;
    bool ligado = 1;
    int tempoDesligado = 0; // sera atualizado pelo tick do sistema (int)
    Tarefa *tarefaAtual = nullptr;

public:
    CPU(int id);
    ~CPU();
    void ocupar();                           // emUso = true;
    void liberar();                          // emUso = FALSE;
    void ligar();                            // ligado = true;
    void desligar();                         // ligado = false;
    void incrementaTempoDesligado(int tick); // sera incrementado de acordo com o tick
    void atribuirTarefa(Tarefa *tarefa);     // executa a tarefa, estado para "EXECUTANDO" e ocupa CPU
    void removerTarefa();                    // remove a tarefa atual, estado para "TERMINADA" e libera CPU
    Tarefa *getTarefaAtual() const;
    int estaLivre() const;
    void executarTick();

    // getter e setters
    int getId() const;
    bool getEmUso() const;
    bool getLigado() const;
    int getTempoDesligado() const;
};

// fila de prontos