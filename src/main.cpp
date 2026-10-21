#include "header/simulador.hpp"
#include "header/leitorConfig.hpp"
#include <time.h>
using namespace std;

void testeLeitor();

int main()
{   
    //srand(time(nullptr));
    srand(67); // gera msm sequencia de num aleaotiros
    cout << "Simulador de escalonamento de tarefas periodicas" << endl;
    Simulador simulador(15, 5, "RM");
    simulador.criaCPUS();
    simulador.criaTarefas();
    testeLeitor();
    return 0;
}

/*teste leitor*/
void testeLeitor()
{
    Configuracao config;
    // tenta carregar o arquivo
    bool carregou = carregarConfiguracao("tests/config.txt", config);
    // mostra os erros encontrados
    for (string erro : config.erros)
        cout << "ERRO: " << erro << endl;
    // Mostra os avisos encontrados
    for (string aviso : config.avisos)
        cout << "AVISO: " << aviso << endl;
    // Encerra se houver erros
    if (!carregou)
        return;
    // mostra config lidas
    cout << "Algoritmo: " << config.algoritmo << endl;
    cout << "Quantum: " << config.quantum << endl;
    cout << "CPUs:" << config.qtdeCpus << endl;
    cout << "Tarefas: " << config.tarefas.size() << endl;

    // mostra os dados de cada tarefa
    for (const Tarefa &tarefa : config.tarefas)
    {
        cout << "\nTarefa " << tarefa.getId() << endl;
        cout << "Cor: " << tarefa.getCor() << endl;
        cout << "Ingresso: " << tarefa.getIngresso() << endl;
        cout << "Duracao: " << tarefa.getDuracao() << endl;
        cout << "Periodo: " << tarefa.getPeriodo() << endl;
        cout << "Prazo: " << tarefa.getPrazo() << endl;
    }
}