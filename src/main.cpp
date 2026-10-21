#include "header/simulador.hpp"
#include "header/leitorConfig.hpp"
#include "header/interfaceGrafica.hpp"
#include <time.h>
using namespace std;

bool testeLeitor(const string &caminho, Configuracao &config);

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
    Configuracao config; // guarda o que foi lido do arquivo de configuracao
    // Encerra se o arquivo tiver erros (os erros ja foram mostrados)
    if (!testeLeitor(caminho, config))
        return 1;

    srand(67); // gera msm sequencia de num aleaotiros

    // Cria a janela da aplicação com largura 1200 e altura 750 pixels.
    InitWindow(1200, 750, "Simulador de Escalonamento");

    // Define a taxa de atualização da janela: até 60 quadros por segundo.
    SetTargetFPS(60);

    // Inicializa a integração entre Raylib e Dear ImGui.
    // O parâmetro true ativa o suporte aos recursos de fonte padrão do rlImGui.
    rlImGuiSetup(true);

    // Cria o simulador com os valores lidos do arquivo de configuração.
    Simulador simulador(config.qtdeCpus, config.quantum, config.algoritmo);

    /* ============
    Inicializa os componentes internos da simulação.
    =========================*/
    simulador.definirTarefas(config.tarefas);
    simulador.reiniciar();
    //==============================

    string mensagem = ""; // resultado da ultima execucao completa, mostrado ao usuario
    // O loop principal mantém a janela aberta até que o usuário a feche.
    while (!WindowShouldClose())
    {
        // Inicia o desenho de um novo quadro da janela Raylib.
        BeginDrawing();

        // Define a cor de fundo da janela.
        ClearBackground(Color{24, 27, 34, 255});

        // Inicia o desenho dos componentes ImGui neste quadro.
        rlImGuiBegin();

        // Cria a janela que contém os controles da simulação.
        ImGui::Begin("Controle da simulacao");

        ImGui::Text("Tick atual: %d", simulador.getTickAtual());

        // Button() retorna true somente no quadro em que o botão é clicado.
        if (ImGui::Button("Voltar tick"))
        {
            simulador.retrocedeTick();
            mensagem = ""; // apaga o resultado da execucao completa anterior
        }

        // SameLine() coloca o próximo componente na mesma linha do anterior.
        ImGui::SameLine();

        if (ImGui::Button("Proximo tick"))
        {
            simulador.avancaTick();
            mensagem = "";
        }

        ImGui::SameLine();

        if (ImGui::Button("Resetar"))
        {
            simulador.reiniciar();
            mensagem = "";
        }
        
        ImGui::SameLine();

        if (ImGui::Button("Executar tudo"))
        {
            if (!simulador.executarCompleto())
            {
                // Falha: a simulacao nao terminou dentro do limite de ticks
                mensagem = "Limite de ticks atingido";
            }
            else
            {
                // TODO: gerar a imagem do Gantt completo (requisito 2.4)
                // bool gerou = exportarGantt(simulador, "gantt.svg");
                // if (gerou)
                //     mensagem = "Simulacao concluida. Imagem salva em: gantt.svg";
                // else
                //     mensagem = "Simulacao concluida, mas nao foi possivel salvar a imagem";

                mensagem = "Simulacao concluida no tick " + to_string(simulador.getTickAtual()); // provisorio, ate a exportacao existir
            }
        }
        if (!mensagem.empty())
            ImGui::TextWrapped("%s", mensagem.c_str());

        // Encerra a janela de controles.
        ImGui::End();

        // Desenha o gráfico e a tabela com os dados atualizados do simulador.
        desenharGantt(simulador);
        desenharTarefas(simulador);

        // Finaliza o desenho dos componentes ImGui.
        rlImGuiEnd();

        // Finaliza o quadro da Raylib e apresenta o resultado na janela.
        EndDrawing();
    }

    // Libera os recursos utilizados pela integração gráfica.
    rlImGuiShutdown();

    // Fecha a janela da Raylib.
    CloseWindow();

    return 0;
}

/*teste leitor*/
bool testeLeitor(const string &caminho, Configuracao &config)
{
    bool carregou = carregarConfiguracao(caminho, config);
    // mostra os erros encontrados
    for (string erro : config.erros)
        cout << "ERRO: " << erro << endl;
    // Mostra os avisos encontrados
    for (string aviso : config.avisos)
        cout << "AVISO: " << aviso << endl;
    // Encerra se houver erros
    if (!carregou)
        return false;
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
    return true;
}