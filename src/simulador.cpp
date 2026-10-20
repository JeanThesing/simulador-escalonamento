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
        escalonador = std::make_unique<EscalonadorRM>();

    else if (algoritmo_escalonamento == "EDF")
        escalonador = std::make_unique<EscalonadorEDF>();
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
    tarefas.emplace_back(1, "FF0000", 0, 3, 5, 5);
    std::cout << "Tarefa 1 feita" << std::endl;
    tarefas.emplace_back(2, "00FF00", 0, 4, 10, 10);
    std::cout << "Tarefa 2 feita" << std::endl;

    tarefas.emplace_back(3, "0000FF", 2, 2, 7, 7);
    std::cout << "Tarefa 3 feita" << std::endl;
}
void Simulador::atualizaFilaDeProntas()
{ //A SER IMPLEMENTADO: atualiza a fila de tarefas prontas de acordo com o algoritmo de escalonamento
    //fila de prontos deve ser atualizada a cada tick, para que o escalonador possa selecionar a proxima tarefa a ser executada
    //deve ser atualizada de acordo com rM OU edf
}

void Simulador::avancaTick()
{
    relogio.incrementa();
    executarSimulacao();
}

void Simulador::retrocedeTick() {}
void Simulador::executarSimulacao() {} // chama escalonador, executa tarefas, atualiza tempo restante, estado das tarefas, etc
void Simulador::salvaHistoricoTick()
{
    EstadoSistema estadoAtual; // estado atual do tick(salva tudo)
    estadoAtual.tick = relogio.getTick(); //tick com base no relogio global do sistema
    estadoAtual.tarefas = tarefas; //copia o vetor de tarefas pro historico
    for (const auto &cpu : cpus) //precisa percorrer as cpus, pois a tarefa atual é apontada por ponteiro, e no historico nao podemos salvar ponteiros, entao precisamos salvar o id da tarefa atual
    {
        EstadoCPU estadoCPU;
        estadoCPU.id = cpu.getId();
        estadoCPU.idTarefaAtual = (cpu.getTarefaAtual() != nullptr) ? cpu.getTarefaAtual()->getId() : -1; // pega o id da tarefa da cpu no momento, se nao tiver tarefa, salva -1
        estadoCPU.ligado = cpu.getLigado();
        estadoCPU.emUso = cpu.getEmUso();
        estadoCPU.tempoDesligado = cpu.getTempoDesligado();
        estadoAtual.cpus.push_back(estadoCPU);
    }
    estadoAtual.quantum = quantum; // salva o quantum atual
    estadoAtual.filaDeProntas = std::vector<int>(filaDeProntas); // copia a fila de prontas para o historico
    historico.push_back(estadoAtual);
}

/*struct EstadoSistema //salva o estado do sistema em determinado tick, para permitir retroceder o tempo da simulacao
{
    int tick = 0;

    std::vector<Tarefa> tarefas;
    std::vector<EstadoCPU> cpus;

    std::queue<int> filaDeProntas;

    int quantum = 0;
};*/
