#pragma once
#include <iostream>
#include "escalonador.hpp"

// Prioridade determinada pelo menor prazoAbsoluto

class EscalonadorEDF : public Escalonador
{
public:
    virtual Tarefa *selecionar(
        const std::vector<Tarefa *> &prontas) override; // seleciona proxima tarefa a ser executada, de acordo com a politica de escalonamento
};