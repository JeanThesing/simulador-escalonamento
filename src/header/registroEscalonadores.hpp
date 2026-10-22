#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "escalonador.hpp"

// REGISTRO DE ALGORITMOS DE ESCALONAMENTO (requisito 4.2)
//
// Por que existe: antes, o Simulador e o leitor de configuracao tinham os nomes "RM" e "EDF"
// escritos no proprio codigo (if/else). Para incluir um algoritmo novo era preciso alterar a
// simulacao. Agora o simulador, o leitor do arquivo e a interface grafica so consultam este
// registro: um algoritmo novo e incluido chamando registrarEscalonador() (ou a macro
// REGISTRAR_ESCALONADOR) UMA vez, sem tocar em nenhum outro arquivo.
//
// Exemplo (em um .cpp novo, ex.: escalonadorSjf.cpp):
//     class EscalonadorSJF : public EscalonadorPrioridade { ... };
//     REGISTRAR_ESCALONADOR("SJF", EscalonadorSJF)

// Funcao que cria uma instancia nova do algoritmo.
using FabricaEscalonador = std::function<std::unique_ptr<Escalonador>()>;

// Registra um algoritmo. O nome e guardado em MAIUSCULAS (o arquivo de configuracao nao
// diferencia maiusculas de minusculas - requisito 3.3.2). Registrar o mesmo nome substitui o anterior.
void registrarEscalonador(const std::string &nome, FabricaEscalonador fabrica);

// true se existe algoritmo com esse nome (comparacao sem diferenciar maiusculas/minusculas).
bool algoritmoRegistrado(const std::string &nome);

// Cria o escalonador pelo nome; devolve nullptr se o nome nao estiver registrado.
std::unique_ptr<Escalonador> criarEscalonador(const std::string &nome);

// Nomes de todos os algoritmos registrados (usado pelo combo da interface e nas mensagens de erro).
std::vector<std::string> nomesAlgoritmos();

// Auxiliar da macro: registra no momento da inicializacao do programa.
struct RegistradorEscalonador
{
    RegistradorEscalonador(const std::string &nome, FabricaEscalonador fabrica)
    {
        registrarEscalonador(nome, std::move(fabrica));
    }
};

// Registra a classe 'CLASSE' sob o nome 'NOME' automaticamente ao iniciar o programa.
#define REGISTRAR_ESCALONADOR(NOME, CLASSE)                                          \
    static const RegistradorEscalonador registrador_##CLASSE(                        \
        NOME, []() -> std::unique_ptr<Escalonador> { return std::make_unique<CLASSE>(); })
