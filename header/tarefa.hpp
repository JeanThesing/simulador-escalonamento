#pragma once
#include <iostream>
#include <string>

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
    int idCPU = -1; // id do CPU atual em que a tarefa esta executando
    std::string cor = "000000"; // preto padrao
    EstadoTarefa estado = EstadoTarefa::NOVA;
    /*lista chamada TCB As informações de cada tarefa (antes, durante, depois da simulação) devem ser
    armazenadas durante a simulação em uma única estrutura de dados, p.ex., Task
    Control Block (TCB);*/
    int ingresso; // indica o instante de tempo que a tarefa foi criada;
    int duracao;  // indica o tempo de execução da tarefa;
    int periodo;  /*indica o período no qual uma tarefa é ativada: valores maiores que
zero indicam que a tarefa tem ativação periódica a cada “n” ticks de relógio; valor
zero indica que a tarefa é aperiódica, ou seja, é ativada apenas uma vez no momento
da sua criação; valores menores que zero representam erros no arquivo.*/
    int prazo;    /*indica o tempo máximo que uma ocorrência/execução da tarefa deve
terminar a sua execução (deadline). O prazo começa a contar a partir do momento
que a tarefa for ativada. Caso o tempo de execução da ocorrência/execução da tarefa
ultrapasse o prazo (deadline), isso indica um erro e deve ser mostrado no gráfico de
gantt. A tarefa que perdeu o prazo continua sendo executada e só termina quando o
tempo de execução da tarefa for igual ou maior que a duração da tarefa.*/
    /*lista_eventos indica uma lista de eventos que ocorre durante a execução
da tarefa.
*/
public:
    Tarefa(
        int id,
        std::string cor,
        int ingresso,
        int duracao,
        int periodo,
        int prazo
    );
    ~Tarefa();
    int getId() const;
    void setId(int id);
    EstadoTarefa getEstado() const;
    void setEstado(EstadoTarefa estado);
    int getIngresso() const;
    void setIngresso(int ingresso);
    int getDuracao() const;
    void setDuracao(int duracao);
    int getPeriodo() const;
    void setPeriodo(int periodo);
    int getPrazo() const;
    void setPrazo(int prazo);
    std::string getCor() const;
    void setCor();
    int getIdCPU() const;
    void setIdCPU(int idCPU);
};