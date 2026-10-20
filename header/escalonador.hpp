#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include "tarefa.hpp"
#include "cpu.hpp"

class Escalonador {
public:
    virtual void escalonar(
        std::vector<Tarefa>& tarefas,
        std::vector<CPU>& cpus,
        std::queue<int>& filaDeProntas
    ) = 0;

    virtual ~Escalonador() = default;
};