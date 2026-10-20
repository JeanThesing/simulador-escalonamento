#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "tarefa.hpp"

class Escalonador
{
public:
    virtual ~Escalonador() = default;

    /*o escalonador é apenas uma função que retorna qual é a
    próxima tarefa a ser executada*/
    Tarefa *selecionar(
        const std::vector<Tarefa *> &prontas); // seleciona proxima tarefa a ser executada, de acordo com a politica de escalonamento

protected:
    // mecanismo de prioridade:

    /*Cada classe filha implementa sua propria funcao de prioridade, retornando qual a prioridade de cada tarefa*/
    virtual int prioridade(const Tarefa *t) const = 0;
    
    // mecanismo de desempate:

    //  escolhe entre duas tarefas com mesma prioridade
    Tarefa* desempatar(Tarefa *a, Tarefa *b);
};
