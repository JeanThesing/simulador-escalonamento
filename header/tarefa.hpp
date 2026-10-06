#pragma once
#include <iostream>

enum class EstadoTarefa
{
    NOVA,
    PRONTA,
    EXECUTANDO,
    SUSPENSA,
    TERMINADA
};

class Tarefa
{
private:
    int id;
    EstadoTarefa estado;
    /*lista chamada TCB As informações de cada tarefa (antes, durante, depois da simulação) devem ser
    armazenadas durante a simulação em uma única estrutura de dados, p.ex., Task
    Control Block (TCB);*/
    int ingresso;
    int duracao;
    int ocorrencia;

public:
};