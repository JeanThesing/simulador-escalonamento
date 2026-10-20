#include "header/cpu.hpp"

CPU::CPU(int id) { this->id = id; }
CPU::~CPU() {} 
void CPU::ocupar() { emUso = true; }

void CPU::liberar()
{
    if(estaLivre()) // se a cpu ja esta livre, nao faz nada
        return;
        
    if (tarefaAtual != nullptr)
    {
        tarefaAtual->setIdCPU(-1);
    }
    if (tarefaAtual != nullptr && tarefaAtual->getEstado() != EstadoTarefa::TERMINADA)
    {
        tarefaAtual->setEstado(EstadoTarefa::PRONTA); //se a tarefa for retirada da cpu, mas, nao necessariamente foi terminada, ela volta para o estado de pronta
    }

    tarefaAtual = nullptr;
    emUso = false;
}

void CPU::ligar() { ligado = true; }

void CPU::desligar() { ligado = false; }

void CPU::incrementaTempoDesligado(int tick) { tempoDesligado += tick; }

void CPU::atribuirTarefa(Tarefa *tarefa)
{
    if (tarefa == nullptr) // se tarefa nao existe, n faz nada
        return;

    if (!estaLivre()) // se a cpu esta ocupada, retorna
        return;

    if (tarefa->getEstado() != EstadoTarefa::PRONTA)
    {
        std::cout << "A tarefa não está pronta para a execução." << std::endl;
        return;
    }
    // Quando começa a executar uma tarefa que está pronta, deixa a CPU ocupada e muda o estado da tarefa como "EXECUTANDO"
    ligar();
    ocupar();

    tarefaAtual = tarefa;
    tarefa->setEstado(EstadoTarefa::EXECUTANDO);
    tarefa->setIdCPU(id); // passa o id da CPU para a tarefa
}

void CPU::removerTarefa()
{
    if (tarefaAtual == nullptr)
        return;

    tarefaAtual->setEstado(EstadoTarefa::TERMINADA);
    liberar();
}

Tarefa *CPU::getTarefaAtual() const
{
    return tarefaAtual;
}

int CPU::estaLivre() const
{
    return !emUso;
}

void CPU::executarTick(){
    if(tarefaAtual == nullptr){
        return;
    }
    tarefaAtual->executarTick(); // decrementa o tempo restante da tarefa
    if (tarefaAtual->getTempoRestante() == 0)
    {
        tarefaAtual->setEstado(EstadoTarefa::TERMINADA);
        liberar();
    }
}

// getters e setters:
int CPU::getId() const {
    return id;
}

bool CPU::getEmUso() const {
    return emUso;
}

bool CPU::getLigado() const {
    return ligado;
}

int CPU::getTempoDesligado() const {
    return tempoDesligado;
}