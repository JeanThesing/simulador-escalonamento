#include "header/tarefa.hpp"
#define LIMITE_EXECUCOES 10
using namespace std;

// builder
Tarefa::Tarefa(int id,
               std::string cor,
               int ingresso,
               int duracao,
               int periodo,
               int prazo)
{
    this->id = id;
    this->cor = cor;
    this->ingresso = ingresso;
    this->duracao = duracao;
    this->tempoRestante = duracao;
    this->periodo = periodo;
    this->prazo = prazo;
    this->estado = EstadoTarefa::NOVA;
    this->idCPU = -1;
}

Tarefa::~Tarefa() {}

// comportamento
void Tarefa::executarTick()
{
    if (estado != EstadoTarefa::EXECUTANDO || tempoRestante <= 0)
        return; // se a tarefa não estiver em execução ou já tiver terminado, não faz nada

    tempoRestante--;

    if (tempoRestante == 0)
    {
        execucoes++;
        idCPU = -1;

        if (execucoes >= LIMITE_EXECUCOES)
            estado = EstadoTarefa::TERMINADA; // tarefa atingiu o limite de execucoes (10)
        else
            estado = EstadoTarefa::AGUARDANDO; //provisorio (tarefa terminou a execucao, mas ainda nao atingiu o limite de execucoes, entao vai aguardar o proximo tick de ativacao)
    }
}

bool Tarefa::deveExecutar(int tick) const
{
    // casos em que a tarefa não deve executar:
    if (periodo < 0){ // se o periodo for mennor q 0 nao pode realizar a divisao por modulo, alem de ser erro
        return false;
        std::cout << "Periodo da tarefa " << id << " é menor que zero. Isso é um erro." << std::endl;
    }

    if (periodo == 0){ // se o periodo for 0 nao pode realizar a divisao por modulo, alem de ser a definicao de uma tarefa aperiodica.
        return false;
        std::cout << "Periodo da tarefa " << id << " é igual a zero. A Tarefa é aperiódica , por isso, será ignorada." << std::endl;
    }

    if (execucoes >= LIMITE_EXECUCOES)
        return false;

    if (tick < ingresso) // Impede que a tarefa seja ativada antes do instante de ingresso.
        return false;

    if (estado == EstadoTarefa::EXECUTANDO ||
        estado == EstadoTarefa::PRONTA ||
        estado == EstadoTarefa::SUSPENSA)
        return false; // Evita que a tarefa seja ativada novamente enquanto já estiver em execução, pronta ou suspensa.

    return (tick - ingresso) % periodo == 0; // verifica se o tick atual corresponde a um instante de ativação periódica. (modulo pra saber se o tick é multiplo do periodo, pra saber se deve ativar ou nao)
}

void Tarefa::ativar(int tick)
{
    tempoRestante = duracao;
    prazoAbsoluto = tick + prazo; // prazo da tarefa de acordo com o prazo base e o tick atual (deadline absoluto)
    perdeuPrazo = false;
    idCPU = -1;

    estado = EstadoTarefa::PRONTA; // tarefa esta pronta (entra na fila de prontas)
    ativacoes++;
}

void Tarefa::verificarPrazo(int tick)
{
    if (prazoAbsoluto < 0 || tempoRestante <= 0) // se o prazo absoluto não foi definido ou a tarefa já terminou, n faz nada
        return;

    if (tick >= prazoAbsoluto)
        perdeuPrazo = true;
}

/*===============================================================================================*/
// Getters de config imutavel
int Tarefa::getId() const { return id; }

const string &Tarefa::getCor() const { return cor; }

int Tarefa::getIngresso() const { return ingresso; }

int Tarefa::getDuracao() const { return duracao; }

int Tarefa::getPeriodo() const { return periodo; }

int Tarefa::getPrazo() const { return prazo; }

// getters de estado de execucao
EstadoTarefa Tarefa::getEstado() const { return estado; }

int Tarefa::getIdCPU() const { return idCPU; }

int Tarefa::getTempoRestante() const { return tempoRestante; }

int Tarefa::getPrazoAbsoluto() const { return prazoAbsoluto; }

int Tarefa::getExecucoes() const { return execucoes; }

int Tarefa::getAtivacoes() const { return ativacoes; }

bool Tarefa::getPerdeuPrazo() const { return perdeuPrazo; }

// setters de estado de execucao
void Tarefa::setEstado(EstadoTarefa estado) { this->estado = estado; }

void Tarefa::setIdCPU(int idCPU) { this->idCPU = idCPU; }

void Tarefa::setTempoRestante(int tempoRestante) { this->tempoRestante = tempoRestante; }