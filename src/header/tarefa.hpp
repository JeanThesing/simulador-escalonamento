#pragma once
#include <iostream>
#include <string>

enum class EstadoTarefa
{
    NOVA,
    PRONTA,
    EXECUTANDO,
    SUSPENSA,
    AGUARDANDO, //(tarefa terminou a execucao, mas ainda nao atingiu o limite de execucoes, entao vai aguardar o proximo tick de ativacao)
    TERMINADA
};

class Tarefa
{
private:
    int id;
    int idCPU = -1;             // id do CPU atual em que a tarefa esta executando
    std::string cor = "000000"; // preto padrao
    EstadoTarefa estado = EstadoTarefa::NOVA;
    /*lista chamada TCB As informações de cada tarefa (antes, durante, depois da simulação) devem ser
    armazenadas durante a simulação em uma única estrutura de dados, p.ex., Task
    Control Block (TCB);*/
    int ingresso;      // indica o instante de tempo que a tarefa foi criada;
    int duracao;       // indica o tempo de execução da tarefa (tempo de computacao);
    int tempoRestante; // tempo restante da execucao da tarefa. Base é = duracao.
    int periodo;       /*intervalo de tempo entre as ativações da tarefa*/
    int prazo;         /*SEMPRE MENOR QUE O periodo, e maior que o tempo de computacao*/
    /*indica o tempo máximo que uma ocorrência/execução da tarefa deve
     terminar a sua execução (deadline). O prazo começa a contar a partir do momento
     que a tarefa for ativada. Caso o tempo de execução da ocorrência/execução da tarefa
     ultrapasse o prazo (deadline), isso indica um erro e deve ser mostrado no gráfico de
     gantt. A tarefa que perdeu o prazo continua sendo executada e só termina quando o
     tempo de execução da tarefa for igual ou maior que a duração da tarefa.*/
    /*lista_eventos indica uma lista de eventos que ocorre durante a execução
    da tarefa.*/
    int prazoAbsoluto = -1; // prazo absoluto da tarefa, calculado a partir do tick de ingresso + prazo relativo
    int execucoes = 0;      // a tarefa finaliza quando executa 10 vezes (por ser periodica)
    int ativacoes = 0;      // quantidade de vezes que a tarefa foi ativada (para ser usada no gantt)
    /*bool deveExecutar = false; // indica se a tarefa deve executar ou nao, para ser usado no gantt
    bool terminouExecucao = false; // indica se a tarefa terminou a execucao ou nao*/
    bool perdeuPrazo = false; // indica se a tarefa perdeu o prazo ou nao, para ser usado no gantt
public:
    Tarefa(
        int id,
        std::string cor,
        int ingresso, // passado pelo relogio do sistema
        int duracao,
        int periodo,
        int prazo);
    ~Tarefa();

    // comportamento
    void executarTick();
    bool deveExecutar(int tick) const;
    void ativar(int tick); // ativa a tarefa, setando o estado para PRONTA e calculando o prazo absoluto
    void verificarPrazo(int tick);

    // Getters de configuração (imutável)
    int getId() const;
    const std::string &getCor() const;
    int getIngresso() const;
    int getDuracao() const;
    int getPeriodo() const;
    int getPrazo() const;

    // Getters de estado de execução
    //  estado
    EstadoTarefa getEstado() const;
    // idcpu
    int getIdCPU() const;
    // tempo restante
    int getTempoRestante() const;
    int getPrazoAbsoluto() const;
    int getExecucoes() const;
    int getAtivacoes() const;
    bool getPerdeuPrazo() const;

    // --- Setters: somente o que realmente muda de fora ---
    // estado
    void setEstado(EstadoTarefa estado);
    // idcpu
    void setIdCPU(int idCPU);
    // tempo restante
    void setTempoRestante(int tempoRestante);
};


/*
Tempo de chegada (Arrival Time ai): instante em que o escalonador toma conhecimento de uma nova ativação da tarefa.

Tempo de liberação (Release Time ri): instante em que essa ativação é inserida na fila de Prontos, ficando disponível para ser escolhida pelo escalonador.


Release Jitter (Ji), que representa a variação máxima dos tempos de liberação
*/

/*
Chegada: o sistema sabe que a tarefa precisa executar
Liberação: a tarefa está pronta e pode disputar a CPU
Início: a tarefa efetivamente recebe a CPU
*/