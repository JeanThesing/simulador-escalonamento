#include "header/escalonadorRm.hpp"

// Prioridade determinada pelo menor periodo
// Rate Monotonic (RM)
// preemptivo e estatico

/*Prioridade e determinada pelo período (Pi)
. Quanto menor o período, maior a prioridade*/

/*
. Premissas
· Tarefas periódicas e independentes
. Di = Pi (Deadline igual ao periodo)
. Ci (tempo de computacao (duracao)) conhecido e constante
· Tempo de chaveamento entre tarefas é nulo (troca de contexto)
*/

bool EscalonadorRM::temMaiorPrioridade(Tarefa *a, Tarefa *b, Tarefa* tarefaAnterior)
{
    return a->getPeriodo() < b->getPeriodo(); // retorna true se A tem menor periodo q B
}

Tarefa* EscalonadorRM::desempatar(Tarefa *a, Tarefa *b, Tarefa* tarefaAnterior){
    if (tarefaAnterior == a)
        return a;
    else if (tarefaAnterior == b)
        return b;
    else
        return a; // desempate arbitrario, retorna A
}

Tarefa *EscalonadorRM::selecionar(const std::vector<Tarefa *> &prontas)
{
    {
        Tarefa *escolhida = nullptr; // ponteiro para a tarefa com maior prioridade (menor período)

        for (Tarefa *tarefa : prontas)
        {
            if (escolhida == nullptr ||
                temMaiorPrioridade(tarefa, escolhida))
            {
                escolhida = tarefa;
            }
        }

        return escolhida;
    }
}

/*
1. Teste de escalonabilidade: determina se as restrições
temporais de um conjunto de tarefas serão atendidas
considerando os critérios de ordenação definidos no
algoritmo de escalonamento

2. Escalonamento: é a própria execução das tarefas
seguindo o escalonamento
*/
