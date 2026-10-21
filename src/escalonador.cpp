#include "header/escalonador.hpp"
#include <cstdlib>

Tarefa *Escalonador::selecionar(const std::vector<Tarefa *> &prontas)
{
    sorteou = false; // zera a cada chamada de selecionar
    std::vector<Tarefa *> empatadas; // tarefas empatadas na primeira posicao

    for (Tarefa *t : prontas)
    {
        if (empatadas.empty())
            empatadas.push_back(t); // primeira tarefa da lista
        else
        {
            int resultado = comparar(t, empatadas[0]); // se nao estiver vazia, compara a proxtarefa com a primeira da lista

            if (resultado > 0)
            {
                empatadas.clear(); //esvazia a lista pois a nova tarefa tem prio maior
                empatadas.push_back(t); // t tem prioridade maior que as ja escolhidas
            }
            else if (resultado == 0)
                empatadas.push_back(t); // se tiver empate total ele a coloca na lista de empatadas
        }
    }

    if (empatadas.empty())
        return nullptr; // nao tinha tarefas prontas

    if (empatadas.size() == 1) //se so tiver uma tarefa
        return empatadas[0];

    // 5- sorteio entre todas as empatadas (mesma chance para cada uma)
    sorteou = true;
    return empatadas[rand() % empatadas.size()];
}

// prioridade do algoritmo. se empatar, aplica o desempate
// retorna: >0 se a vence, <0 se b vence, 0 se empate total
int Escalonador::comparar(const Tarefa *a, const Tarefa *b) const
{
    if (prioridade(a) != prioridade(b))
        return prioridade(a) > prioridade(b) ? 1 : -1; //1 se a vence, -1 se b vence

    return desempatar(a, b); //1 se a vence, -1 se b vence, e 0 se empate total (sorteio)
}

int Escalonador::desempatar(const Tarefa *a, const Tarefa *b) const
{
    // 1- a que ja estava executando (evita troca de contexto da msm tarefa)
    bool aExecutando = (a->getEstado() == EstadoTarefa::EXECUTANDO);
    bool bExecutando = (b->getEstado() == EstadoTarefa::EXECUTANDO);
    if (aExecutando && !bExecutando) return 1;
    if (bExecutando && !aExecutando) return -1;

    // 2- menor prazo
    if (a->getPrazo() < b->getPrazo()) return 1;
    if (b->getPrazo() < a->getPrazo()) return -1;
    
    // 3- quem ingressou antes
    if (a->getIngresso() < b->getIngresso()) return 1;
    if (b->getIngresso() < a->getIngresso()) return -1;

    // 4- menor duracao
    if (a->getDuracao() < b->getDuracao()) return 1;
    if (b->getDuracao() < a->getDuracao()) return -1;

    // 5- empate total: o sorteio é feito no selecionar()
    return 0;
}

/*4.3. Em caso de empates na escolha da próxima tarefa (para todos os algoritmos), o
desempate deve considerar os seguintes critérios, nessa ordem: (1) a tarefa selecionada
é a que está executando imediatamente antes do escalonamento ser acionado (para
evitar uma troca de contexto para a própria tarefa); (2) o prazo da tarefa; (3) o instante
de ingresso da tarefa: quem chegou antes é escolhida; (4) duração da tarefa: duração
menor deve se escolhida; (5) sorteio (quando isso acontecer, um elemento gráfico deve
ser mostrado no gráfico de gantt no momento em que a tarefa escolhida (re)começar
a execução).*/