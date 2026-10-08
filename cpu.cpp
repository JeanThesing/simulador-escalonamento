#include "header/cpu.hpp"

CPU::CPU(int id) { this->id = id; }
CPU::~CPU() {}
void CPU::ocupar(){ emUso = true; }

void CPU::liberar() { emUso = false; }

void CPU::ligar() { ligado = true; }

void CPU::desligar() { ligado = false; }

void CPU::incrementaTempoDesligado(int tick) { tempoDesligado += tick; }

void CPU::executarTarefa(Tarefa *tarefa)
{
    if (tarefa->getEstado() != EstadoTarefa::PRONTA)
    {
        std::cout << "Não tá pronta porra" << std::endl;
        return;
    }
    // Quando começa a executar uma tarefa que está pronta, deixa a CPU ocupada e muda o estado da tarefa como "EXECUTANDO"
    ligar();
    ocupar();
    tarefa->setEstado(EstadoTarefa::EXECUTANDO);
}