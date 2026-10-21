#pragma once
#include "escalonador.hpp"

class EscalonadorPrioridade : public Escalonador
{
private:
    bool sorteou = false; //flag que indica se houve sorteio na ultima chamada de selecionar()

protected:
    // mecanismo de prioridade:

    /*Cada classe filha implementa sua propria funcao de prioridade, retornando qual a prioridade de cada tarefa*/
    virtual int prioridade(const Tarefa *t) const = 0;

    // mecanismo de desempate:
    // compara duas tarefas: >0 se A vence, <0 se B vence, 0 se empate total
    int comparar(const Tarefa *a, const Tarefa *b) const;
    // aplica os criterios de desempate: >0 se A vence, <0 se B vence, 0 se empate total
    int desempatar(const Tarefa *a, const Tarefa *b) const;

public:
    /*o escalonador é apenas uma função que retorna qual é a
    próxima tarefa a ser executada*/
    Tarefa *selecionar(
        const std::vector<Tarefa *> &prontas) override; // seleciona proxima tarefa a ser executada, de acordo com a politica de escalonamento

    // true se a ultima chamada de selecionar() precisou de sorteio (criterio 5 de desempate)
    // O simulador consulta dps de selecionar() para marcar o evento no grafico gant
    bool houveSorteio() const override { return sorteou; }
};