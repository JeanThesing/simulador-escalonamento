#pragma once
#include <iostream>
#include "escalonador.hpp"

class EscalonadorRM : public Escalonador
{
public:
    virtual Tarefa *selecionar(
        const std::vector<Tarefa *> &prontas) override; // seleciona proxima tarefa a ser executada, de acordo com a politica de escalonamento
    bool temMaiorPrioridade(Tarefa *a, Tarefa *b, Tarefa* tarefaAnterior);
    Tarefa* desempatar(Tarefa *a, Tarefa *b, Tarefa* tarefaAnterior);
};