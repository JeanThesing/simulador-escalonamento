#include "header/tarefa.hpp"
#define LIMITE_EXECUCOES 10
using namespace std;

int Tarefa::limiteExecucoes() { return LIMITE_EXECUCOES; }

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
    this->proximaAtivacao = ingresso; // a primeira liberacao acontece no instante de ingresso
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

    tempoRestante--; //diminui o tempo restante da tarefa a cada tick

    // se o tempo restante for 0, a tarefa terminou a execução e deve ser marcada como AGUARDANDO ou TERMINADA (se completou todas as 10)
    if (tempoRestante == 0)
    {
        execucoes++;
        idCPU = -1; // a tarefa terminou de executar, entao nao esta mais em nenhuma cpu

        if (execucoes >= LIMITE_EXECUCOES)
            estado = EstadoTarefa::TERMINADA; // tarefa atingiu o limite de execucoes (10)
        else
            estado = EstadoTarefa::AGUARDANDO; //(tarefa terminou a execucao, mas ainda nao atingiu o limite de execucoes, entao vai aguardar o proximo tick de ativacao)
    }
}

// define se tarefa deve ser ativada no tick
// chama no avancaTick() do simulador, uma vez por tick para cada taref
bool Tarefa::deveExecutar(int tick) const
{
    // casos em que a tarefa não deve executar:
    if (periodo <= 0) // aperiódica (0) ou erro (<0): o aviso é dado na leitura do arquivo
    {
        return false;
    }

    if (execucoes >= LIMITE_EXECUCOES) // execucoes >= 10: a tarefa ja terminou (periodica)
        return false;

    // a instancia anterior ainda esta viva
    // a tarefa so tem uma instancia por vez se ainda esta pronta, executando ou suspensa
    // a proxima ativacao tem que esperar.
    // deve aguradar quando a instancia anterior perdeu o prazo: ela continua ate terminar (requisito 3.3) e so depois a proxima entra.
    if (estado == EstadoTarefa::EXECUTANDO ||
        estado == EstadoTarefa::PRONTA ||
        estado == EstadoTarefa::SUSPENSA)
        return false; // a instancia anterior ainda nao terminou (mesmo que tenha perdido o prazo): a proxima espera

    // verifica se chegou a hora da prox ativacao
    // >= para permitir ativações atrasadas caso a tarefa anterior ainda estivesse executando
    // se a instancia anterior atrasou, a proxima e ativada assim que ela terminar (modulo so permitia no instante de multiplos do periodo)
    return tick >= proximaAtivacao;
}

void Tarefa::ativar(int tick)
{
    tempoRestante = duracao;
    prazoAbsoluto = tick + prazo; // prazo da tarefa de acordo com o prazo base e o tick atual (deadline absoluto)
    proximaAtivacao += periodo; // Define o instante da próxima ativação da tarefa.
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

int Tarefa::getProximaAtivacao() const { return proximaAtivacao; }

// setters de estado de execucao
void Tarefa::setEstado(EstadoTarefa estado) { this->estado = estado; }
 
void Tarefa::setIdCPU(int idCPU) { this->idCPU = idCPU; }
 
void Tarefa::setTempoRestante(int tempoRestante) { this->tempoRestante = tempoRestante; }
 
// Setters das características (usados pelo editor de tarefas; a validação é feita em Simulador::editarTarefa)
void Tarefa::setCor(const string &cor) { this->cor = cor; }
 
void Tarefa::setIngresso(int ingresso) { this->ingresso = ingresso; }
 
void Tarefa::setDuracao(int duracao) { this->duracao = duracao; }
 
void Tarefa::setPeriodo(int periodo) { this->periodo = periodo; }
 
void Tarefa::setPrazo(int prazo) { this->prazo = prazo; }
 
void Tarefa::setPrazoAbsoluto(int prazoAbsoluto) { this->prazoAbsoluto = prazoAbsoluto; }
 
void Tarefa::setProximaAtivacao(int proximaAtivacao) { this->proximaAtivacao = proximaAtivacao; }