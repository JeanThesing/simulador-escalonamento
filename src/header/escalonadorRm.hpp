#pragma once
#include <iostream>
#include "escalonador.hpp"

// Prioridade determinada pelo menor periodo
// Rate Monotonic (RM)
// preemptivo e estatico

class EscalonadorRM : public Escalonador
{
protected:
    /*Prioridade e determinada pelo período (Pi)
    . Quanto menor o período, maior a prioridade*/

    // menor periodo = maior prioridade, por isso o sinal negativo
    virtual int prioridade(const Tarefa *t) const override
    {
        return -(t->getPeriodo());
    }
};


/*
. Premissas
· Tarefas periódicas e independentes
. Di = Pi (Deadline igual ao periodo)
. Ci (tempo de computacao (duracao)) conhecido e constante
· Tempo de chaveamento entre tarefas é nulo (troca de contexto)

fonte: https://www.youtube.com/watch?v=Dy0Bac4sH4U
*/