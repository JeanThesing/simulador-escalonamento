#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "tarefa.hpp"

// Interface de qualquer algoritmo de escalonamento (requisito 4.2).
// O simulador so conhece esta classe: para incluir um algoritmo novo basta
// criar uma classe filha, sem mexer no codigo da simulacao
class Escalonador
{
public:
    virtual ~Escalonador() = default;

    // Recebe as tarefas candidatas e devolve a escolhida (ou nullptr para deixar a CPU ociosa)
    /*o escalonador é apenas uma função que retorna qual é a
    próxima tarefa a ser executada*/
    virtual Tarefa *selecionar(const std::vector<Tarefa *> &prontas) = 0;

    // Por padrao nao ha sorteio. Quem sorteia (EscalonadorPrioridade) sobrescreve.
    virtual bool houveSorteio() const { return false; }
};