#include "header/simulador.hpp"
using namespace std;

Simulador::Simulador(int qtde_cpus, int quantum, std::string algoritmo_escalonamento)
{
    this->qtde_cpus = qtde_cpus;
    this->quantum = quantum;
    this->algoritmo_escalonamento = algoritmo_escalonamento;
}

Simulador::~Simulador() {}

void Simulador::criaEscalonador()
{
    if (algoritmo_escalonamento == "RM")
    {
        escalonador = EscalonadorRM();
    }
    if (algoritmo_escalonamento == "EDF")
    {
        escalonador = EscalonadorEDF();
    }
}

void Simulador::criaCPUS()
{
    cpus = {};
    for (int i = 0; i < qtde_cpus; i++)
    {
        cpus.push_back(CPU(i));
        std::cout << "CPU " << i << " feita" << std::endl;
    }
}

void Simulador::criaTarefas()
{
}

void Simulador::avancaTick()
{
    relogio.incrementa();
    executarSimulacao();
}

void Simulador::retrocedeTick() {}
void Simulador::executarSimulacao() {} // chama escalonador, executa tarefas, atualiza tempo restante, estado das tarefas, etc
void Simulador::salvaHistoricoTick(){
    EstadoSistema estadoAtual; // estado atual do tick(salva tudo)
    estadoAtual.tick = relogio.getTick();
    estadoAtual.tarefas = tarefas;
    estadoAtual.cpus = cpus;
    estadoAtual.quantum = quantum;
    
    historico.push_back(estadoAtual);
}

int main()
{
    Simulador simulador(15, 5, "RM");
    simulador.criaCPUS();
    return 0;
}