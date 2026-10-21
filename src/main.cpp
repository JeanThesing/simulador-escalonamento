#include "header/simulador.hpp"
#include "header/leitorConfig.hpp"
#include <time.h>
using namespace std;

void testeLeitor(const string &caminho);

int main(int argc, char *argv[])
{
    string caminho;
    if (argc > 1)
        caminho = argv[1]; // 1º argumento da linha de comando
    else
    {
        cout << "Arquivo de configuracao: ";
        getline(cin, caminho); // sem argumento: pergunta ao usuario
    }   
    // srand(time(nullptr));
    srand(67); // gera msm sequencia de num aleaotiros
    cout << "Simulador de escalonamento de tarefas periodicas" << endl;
    //Simulador simulador(15, 5, "RM");
    testeLeitor(caminho);
    return 0;
}

/*teste leitor*/
void testeLeitor(const string &caminho)
{
    Configuracao config;
    bool carregou = carregarConfiguracao(caminho, config);
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