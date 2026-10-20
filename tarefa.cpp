#include "header/tarefa.hpp"
using namespace std;

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
int Tarefa::getId() const { return id; }

void Tarefa::setId(int id) { this->id = id; }

EstadoTarefa Tarefa::getEstado() const { return estado; }

void Tarefa::setEstado(EstadoTarefa estado) { this->estado = estado; }

int Tarefa::getIngresso() const { return ingresso; }

void Tarefa::setIngresso(int ingresso) { this->ingresso = ingresso; }

int Tarefa::getDuracao() const { return duracao; }

void Tarefa::setDuracao(int duracao) { this->duracao = duracao; }

int Tarefa::getPeriodo() const { return periodo; }

void Tarefa::setPeriodo(int periodo) { this->periodo = periodo; }

int Tarefa::getPrazo() const { return prazo; }

void Tarefa::setPrazo(int prazo) { this->prazo = prazo; }

string Tarefa::getCor() const { return cor; }

void Tarefa::setCor(const string &cor) { this->cor = cor; }

int Tarefa::getIdCPU() const { return idCPU; }

void Tarefa::setIdCPU(int idCPU) { this->idCPU = idCPU; }

int Tarefa::getTempoRestante() const { return tempoRestante; }

void Tarefa::setTempoRestante(int tempoRestante) { this->tempoRestante = tempoRestante; }

void Tarefa::executarTick()
{
    if (estado == EstadoTarefa::EXECUTANDO && tempoRestante > 0)
    {
        tempoRestante--;
    } // se a tarefa estiver executando e o tempo restante maior q 0, decremente o tempo restante
}

bool Tarefa::deveExecutar(int tick) const
{
    return (execucoes < 10) && //Verifica se a tarefa ainda não alcançou o limite de dez ativações.
           (tick >= ingresso) && //Impede que a tarefa seja ativada antes do instante de ingresso.
           ((tick - ingresso) % periodo == 0); //verifica se o tick atual corresponde a um instante de ativação periódica. (modulo pra saber se o tick é multiplo do periodo, pra saber se deve ativar ou nao)

}

void Tarefa::ativar(int tick) {
    tempoRestante = duracao;
    prazoAbsoluto = tick + prazo; //prazo da tarefa de acordo com o prazo base e o tick atual (deadline absoluto)
    estado = EstadoTarefa::PRONTA; //tarefa esta pronta (entra na fila de prontas)
    execucoes++;
}