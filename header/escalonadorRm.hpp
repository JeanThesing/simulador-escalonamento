#pragma once
#include <iostream>
#include "escalonador.hpp"

class EscalonadorRM: public Escalonador{
public:
    void escalonar(
        std::vector<Tarefa>& tarefas,
        std::vector<CPU>& cpus,
        std::queue<int>& filaDeProntas
    ) override;
};