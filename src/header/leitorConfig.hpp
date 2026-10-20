#pragma once
#include <string>
#include <vector>
#include "tarefa.hpp"

// Resultado da leitura do arquivo de configuracao.
// Os valores iniciais sao os padroes (requisito 3.2) e o arquivo os sobrescreve
struct Configuracao
{
    std::string algoritmo = "RM"; // RM ou EDF q e sempre em maiusculo apos a leitura
    int quantum = 1;
    int qtdeCpus = 1;
    std::vector<Tarefa> tarefas;     // somente tarefas periodicas validas
    std::vector<std::string> avisos; // nao impedem a simulacao (tarefas aperiodicas ignoradas)
    std::vector<std::string> erros;  // se houver algum erro o arquivo nao deve ser usado
};

// Le o arquivo em 'caminho' (qualquer nome/local). Retorna true se nao houve erros.
bool carregarConfiguracao(const std::string &caminho, Configuracao &config);