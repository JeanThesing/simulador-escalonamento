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

    // Semente do proximo sorteio de desempate. O simulador define a semente antes de cada
    // selecionar() a partir do tick e da posicao (CPU), de modo que o mesmo estado do sistema
    // sempre produz o mesmo sorteio. Isso garante que avancar depois de retroceder repita
    // exatamente o que ja tinha sido simulado (requisito 1.5.2). Padrao: ignora (sem sorteio).
    virtual void definirSemente(unsigned long long /*semente*/) {}
};