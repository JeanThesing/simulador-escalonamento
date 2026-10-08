#include "header/tarefa.hpp"
using namespace std;

Tarefa::Tarefa(int id,
               std::string cor,
               int ingresso,
               int duracao,
               int periodo,
               int prazo) {}
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

void Tarefa::setCor() { this->cor = cor; }

int Tarefa::getIdCPU() const{ return idCPU; }

void Tarefa::setIdCPU(int idCPU){ this->idCPU = idCPU;}