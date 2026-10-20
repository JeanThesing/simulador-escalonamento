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

    virtual Tarefa *selecionar(
        const std::vector<Tarefa *> &prontas) = 0; //seleciona proxima tarefa a ser executada, de acordo com a politica de escalonamento
};

// Correção lógica (correctness) Produzir o resultado correto
// Correção temporal (timeliness) Produzir esse resultado dentro do prazo