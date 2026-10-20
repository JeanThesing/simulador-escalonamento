#include "header/escalonador.hpp"

Tarefa *Escalonador::selecionar(const std::vector<Tarefa *> &prontas)
{
    Tarefa *escolhida = nullptr;

    for (Tarefa *t : prontas)
    {
        if (escolhida == nullptr)
            escolhida = t; // primeira tarefa da lista
        else if (prioridade(t) > prioridade(escolhida))
            escolhida = t; // t tem prioridade maior
        else if (prioridade(t) == prioridade(escolhida))
            escolhida = desempatar(escolhida, t); // empate: aplica o 4.3
    }

    return escolhida; // nullptr se nao havia tarefas prontas
}

Tarefa *Escalonador::desempatar(Tarefa *a, Tarefa *b)
{
    // 1- a que ja estava executando (evita troca de contexto da msm tarefa)
    bool aExecutando = (a->getEstado() == EstadoTarefa::EXECUTANDO);
    bool bExecutando = (b->getEstado() == EstadoTarefa::EXECUTANDO);
    if (aExecutando && !bExecutando) return a;
    if (bExecutando && !aExecutando) return b;

    // 2- menor prazo
    if (a->getPrazo() < b->getPrazo()) return a;
    if (b->getPrazo() < a->getPrazo()) return b;

    // 3- quem ingressou antes
    if (a->getIngresso() < b->getIngresso()) return a;
    if (b->getIngresso() < a->getIngresso()) return b;

    // 4- menor duracao
    if (a->getDuracao() < b->getDuracao()) return a;
    if (b->getDuracao() < a->getDuracao()) return b;

    // 5- sorteio: PROVISORIO, por enquanto mantem a (o simulador ainda precisa ser avisado do sorteio para o Gantt)
    return a;
}

/*4.3. Em caso de empates na escolha da próxima tarefa (para todos os algoritmos), o
desempate deve considerar os seguintes critérios, nessa ordem: (1) a tarefa selecionada
é a que está executando imediatamente antes do escalonamento ser acionado (para
evitar uma troca de contexto para a própria tarefa); (2) o prazo da tarefa; (3) o instante
de ingresso da tarefa: quem chegou antes é escolhida; (4) duração da tarefa: duração
menor deve se escolhida; (5) sorteio (quando isso acontecer, um elemento gráfico deve
ser mostrado no gráfico de gantt no momento em que a tarefa escolhida (re)começar
a execução).*/