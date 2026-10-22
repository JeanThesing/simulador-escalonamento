#pragma once
#include <iostream>
#include "tarefa.hpp"
#include "cpu.hpp"
#include "relogio.hpp"
#include "escalonador.hpp"
#include "escalonadorRm.hpp"
#include "escalonadorEdf.hpp"
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include "historico.hpp"

// Dados que podem ser alterados no editor de tarefas
struct EdicaoTarefa
{
    EstadoTarefa estado;
    int tempoRestante;
    int ingresso;
    int duracao;
    int periodo;
    int prazo;
    std::string cor;
};


/*No entanto, com objetivo de terminar a simulação, assume-se que uma
tarefa periódica termina após ela ter executado 10 vezes.*/
// somente tarefas periodicas

// NUM DE PROCESSADORES NA SIMULACAO DEVEM SER ENTRE (valores entre 1 e N)
class Simulador
{
private:
    std::vector<EstadoSistema> historico; // historico do sistema
    int qtde_cpus = 1;
    int quantum;
    std::unique_ptr<Escalonador> escalonador;
    std::string algoritmo_escalonamento = "";
    // algoritmo de escalonamento a ser usado
    Relogio relogio = Relogio();
    std::vector<Tarefa> tarefas;                         // todas as tarefas
    std::vector<CPU> cpus;                               // todas as cpus
    std::vector<int> filaDeProntas = std::vector<int>(); // fila com o ids das tarefas prontas (vector para poder ordenar as tarefas de acordo com o algoritmo de escalonamento)
    std::vector<std::vector<int>> historicoGantt;
    void salvarTickGantt();
    std::vector<Tarefa> tarefasIniciais; // tarefas lidas do arquivo de configuracao
    std::vector<EventoGantt> eventos;    // eventos a serem mostrados no Gantt (chegada, termino, prazo perdido) no instante do tick

public:
    Simulador(int qtde_cpus, int quantum, std::string algoritmo_escalonamento);
    ~Simulador();

    void criaCPUS();
    void criaEscalonador();
    void criaTarefas();
    void definirTarefas(const std::vector<Tarefa>& tarefas);

    void definirAlgoritmo(const std::string& algoritmo);
    void definirQuantidadeCPUs(int quantidade);
    void definirQuantum(int quantum);
    // Permite editar uma tarefa no estado atual; retorna false se a alteração for inválida.
    bool editarTarefa(int id, const EdicaoTarefa &edicao, std::string &erro);
    int getQuantum() const { return quantum; }

    void avancaTick();
    void retrocedeTick();         // funcionara com base no historico do sistema (a ser implementado)
    void atualizaFilaDeProntas(); // atualiza a fila de tarefas prontas de acordo com o algoritmo de escalonamento
    void escalonarCPUs(); // seleciona as tarefas a serem executadas nas cpus
    void executarSimulacao();
    void salvaHistoricoTick();

    int getTickAtual() const;
    const std::vector<std::vector<int>> &getHistoricoGantt() const;
    const std::vector<EstadoSistema> &getHistorico() const { return historico; }
    // O histórico terá esta estrutura: historicoGantt[tick][idCPU] = id da tarefa executada
    // por exemplo:
    // {
    //     {-1, -1}, // Tick 0: ambas ociosas
    //     {  1, -1}, // Tick 1: CPU 0 executou tarefa 1
    //     {  1,  2}  // Tick 2: CPU 0 executou tarefa 1; CPU 1 executou tarefa 2
    // }

    const std::vector<CPU> &getCPUs() const;
    const std::vector<Tarefa> &getTarefas() const;
    const std::vector<EventoGantt> &getEventos() const { return eventos; }
    const std::string &getAlgoritmo() const { return algoritmo_escalonamento; } // usado no titulo do Gantt exportado

    void reiniciar();

    // executar sozinho tudo
    bool todasTerminadas() const;                   // true se nenhuma tarefa tem mais ativacoes a fazer
    bool executarCompleto(int limiteTicks = 10000); // roda ate o fim; false se estourou o limite
};

// fila de prontos