#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include "tarefa.hpp"
#include "cpu.hpp"

class Escalonador
{
public:
    virtual ~Escalonador() = default;

    /*o escalonador é apenas uma função que retorna qual é a
    próxima tarefa a ser executada*/
    virtual Tarefa *selecionar(
        const std::vector<Tarefa *> &prontas) = 0; // seleciona proxima tarefa a ser executada, de acordo com a politica de escalonamento

protected:
    // mecanismos de prioridade:

    /*Cada algoritmo implementa sua propria funcao de prioridade, retornando um valor maior para tarefas com maior prioridade*/
    virtual int prioridade(const Tarefa *t) const = 0;
    
    // mecanismos de desempate:

    //  escolhe entre duas tarefas com mesma prioridade
    Tarefa* desempatar(Tarefa *a, Tarefa *b);
};
