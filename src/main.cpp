#include "header/simulador.hpp"
#include "header/leitorConfig.hpp"
#include "header/interfaceGrafica.hpp"
#include "header/exportadorGantt.hpp"
#include <filesystem>
#include <time.h>
using namespace std;

bool testeLeitor(const string &caminho, Configuracao &config);

// Gera o Gantt da simulacao em SVG (requisito 2.4) e devolve a mensagem para mostrar ao usuario.
// O arquivo e salvo na pasta de onde o programa foi executado; a mensagem mostra o caminho completo
// para o usuario saber onde encontrar a imagem.
static string exportarGantt(const Simulador &simulador, const string &prefixo)
{
    const string arquivo = "gantt.svg";
    string erro;

    if (!exportarGanttSVG(simulador, arquivo, erro))
        return prefixo + "Nao foi possivel salvar a imagem: " + erro;

    error_code ec; // sem excecao: se nao der para obter o caminho absoluto, mostra so o nome
    filesystem::path completo = filesystem::absolute(arquivo, ec);
    return prefixo + "Imagem salva em: " + (ec ? arquivo : completo.string());
}

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
    // Se a configuração for inválida, mantém uma janela aberta para explicar os erros.
    if (!testeLeitor(caminho, config))
    {
        InitWindow(900, 500, "Erro de configuracao - Simulador de Escalonamento");
        SetTargetFPS(60);
        rlImGuiSetup(true);
        while (!WindowShouldClose())
        {
            BeginDrawing();
            ClearBackground(Color{24, 27, 34, 255});
            rlImGuiBegin();
            ImGui::Begin("Nao foi possivel carregar a configuracao");
            ImGui::TextWrapped("Verifique o caminho do arquivo e corrija os problemas abaixo:");
            for (const string &erro : config.erros)
                ImGui::BulletText("%s", erro.c_str());
            for (const string &aviso : config.avisos)
                ImGui::BulletText("Aviso: %s", aviso.c_str());
            ImGui::TextWrapped("Feche esta janela e execute novamente com um arquivo valido.");
            ImGui::End();
            rlImGuiEnd();
            EndDrawing();
        }
        rlImGuiShutdown();
        CloseWindow();
        return 1;
    }

    int algoritmo = (config.algoritmo == "EDF") ? 1 : 0;
    int quantidadeCPUs = config.qtdeCpus;
    int quantum = config.quantum;

    srand(67); // gera msm sequencia de num aleaotiros

    // Cria a janela da aplicação com largura 1200 e altura 750 pixels.
    InitWindow(1600, 900, "Simulador de Escalonamento");

    // Define a taxa de atualização da janela: até 60 quadros por segundo.
    SetTargetFPS(60);

    // Inicializa a integração entre Raylib e Dear ImGui.
    // O parâmetro true ativa o suporte aos recursos de fonte padrão do rlImGui.
    rlImGuiSetup(true);

    // Cria o simulador com os valores lidos do arquivo de configuração.
    Simulador simulador(quantidadeCPUs, quantum, algoritmo == 0 ? "RM" : "EDF");

    /* ============
    Inicializa os componentes internos da simulação.
    =========================*/
    simulador.definirTarefas(config.tarefas);
    simulador.reiniciar();
    //==============================

    string mensagem = ""; // resultado da ultima execucao completa, mostrado ao usuario
    for (const string &aviso : config.avisos)
    {
        if (!mensagem.empty())
            mensagem += "\n";
        mensagem += "Aviso: " + aviso;
    }
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
            // O retrocesso restaura também a configuração que estava ativa naquele tick.
            algoritmo = simulador.getAlgoritmo() == "EDF" ? 1 : 0;
            quantidadeCPUs = simulador.getQuantidadeCPUs();
            quantum = simulador.getQuantum();
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
                // A simulacao terminou: gera automaticamente a imagem do Gantt completo (requisito 2.4)
                mensagem = exportarGantt(simulador,
                                         "Simulacao concluida no tick " + to_string(simulador.getTickAtual()) + ". ");
            }
        }

        ImGui::SameLine();

        // Permite gerar a imagem a qualquer momento (ex.: depois de um passo a passo), com o que ja foi simulado
        if (ImGui::Button("Exportar Gantt"))
            mensagem = exportarGantt(simulador, "");

        if (!mensagem.empty())
            ImGui::TextWrapped("%s", mensagem.c_str());

        ImGui::Separator();

        bool configuracaoAlterada = false;

        // Seleciona o algoritmo de escalonamento.
        const char *algoritmos[] = {"RM", "EDF"};

        ImGui::SetNextItemWidth(100);

        if (ImGui::Combo("Algoritmo", &algoritmo, algoritmos, 2))
            configuracaoAlterada = true;

        ImGui::SameLine();

        // Define a quantidade maxima de CPUs conforme o arquivo de configuracao.
        ImGui::SetNextItemWidth(100);

        int maxCPUs = config.qtdeCpus;

        if (maxCPUs < 1)
            maxCPUs = 1;

        if (ImGui::SliderInt("CPUs", &quantidadeCPUs, 1, maxCPUs))
            configuracaoAlterada = true;

        ImGui::SameLine();

        // Define o quantum pela interface.
        ImGui::SetNextItemWidth(120);
        if (ImGui::SliderInt("Quantum", &quantum, 1, 1000))
            configuracaoAlterada = true;

        // Aplica as alterações no tick atual, sem apagar o histórico já desenhado.
        // A nova configuração passa a valer a partir do próximo tick executado.
        if (configuracaoAlterada)
        {
            simulador.definirAlgoritmo(algoritmo == 0 ? "RM" : "EDF");
            simulador.definirQuantidadeCPUs(quantidadeCPUs);
            simulador.definirQuantum(quantum);
            mensagem = "Configuracao atualizada no tick " + to_string(simulador.getTickAtual()) +
                       ". O historico anterior foi preservado.";
        }

        // Encerra a janela de controles.
        ImGui::End();

        // Desenha o gráfico e a tabela com os dados atualizados do simulador.
        desenharGantt(simulador);
        desenharTarefas(simulador);
        desenharEditorTarefas(simulador, mensagem);
        desenharCPUs(simulador);
        desenharLegenda(simulador);

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