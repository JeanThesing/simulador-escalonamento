#pragma once
#include <iostream>
#include "escalonador.hpp"

// Prioridade determinada pelo menor prazoAbsoluto
// Earliest Deadline First (EDF)
// preemptivo e estatico


class EscalonadorEDF : public Escalonador
{
protected:
    // menor prazoAbsoluto = maior prioridade, por isso o sinal negativo
    virtual int prioridade(const Tarefa *t) const override
    {
        return -(t->getPrazoAbsoluto());
    }
};

/*
· Classificação
· Preemptivo, dinâmico e online

. Prioridade e determinada pelo deadline absoluto
· Quanto menor o deadline absoluto, maior a prioridade

. Premissas
· Tarefas periódicas e independentes
. Di = Pi
. Ci conhecido e constante
· Tempo de chaveamento entre tarefas é nulo

fonte: https://www.youtube.com/watch?v=Dy0Bac4sH4U
*/