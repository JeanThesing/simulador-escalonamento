#include "header/simulador.hpp"
#include "header/leitorConfig.hpp"
#include "header/interfaceGrafica.hpp"
#include "header/exportadorGantt.hpp"
#include <filesystem>
#include <algorithm>
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

    // Algoritmos disponiveis vem do registro (requisito 4.2): um algoritmo novo aparece aqui
    // sozinho, sem alterar o main. 'algoritmo' e o indice do algoritmo escolhido nessa lista.
    vector<string> nomesAlg = nomesAlgoritmos();
    auto indiceDoAlgoritmo = [&](const string &nome)
    {
        auto it = find(nomesAlg.begin(), nomesAlg.end(), nome);
        return it == nomesAlg.end() ? 0 : static_cast<int>(it - nomesAlg.begin());
    };
    int algoritmo = indiceDoAlgoritmo(config.algoritmo);
    int quantidadeCPUs = config.qtdeCpus;
    int quantum = config.quantum;

    // O sorteio de desempate nao usa mais rand(): e derivado do tick (ver Simulador::escalonarCPUs),
    // entao nao ha semente global a configurar.

    // Cria a janela da aplicação com largura 1200 e altura 750 pixels.
    InitWindow(1600, 900, "Simulador de Escalonamento");

    // Define a taxa de atualização da janela: até 60 quadros por segundo.
    SetTargetFPS(60);

    // Inicializa a integração entre Raylib e Dear ImGui.
    // O parâmetro true ativa o suporte aos recursos de fonte padrão do rlImGui.
    rlImGuiSetup(true);

    // Cria o simulador com os valores lidos do arquivo de configuração.
    Simulador simulador(quantidadeCPUs, quantum, config.algoritmo);

    /* ============
    Inicializa os componentes internos da simulação.
    =========================*/
    simulador.definirTarefas(config.tarefas);
    simulador.reiniciar();
    //==============================

    // Caminho digitado na interface para carregar outro arquivo de configuracao (requisito 3.3.4)
    char caminhoNovo[512] = "";
    snprintf(caminhoNovo, sizeof(caminhoNovo), "%s", caminho.c_str());

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
            algoritmo = indiceDoAlgoritmo(simulador.getAlgoritmo());
            quantidadeCPUs = simulador.getQuantidadeCPUs();
            quantum = simulador.getQuantum();
            mensagem = ""; // apaga o resultado da execucao completa anterior
        }

        // SameLine() coloca o próximo componente na mesma linha do anterior.
        ImGui::SameLine();

        if (ImGui::Button("Proximo tick"))
        {
            const bool jaTerminada = simulador.todasTerminadas();
            simulador.avancaTick();
            mensagem = "";

            // Requisito 2.4: ao final da simulacao a imagem do Gantt e gerada automaticamente,
            // tambem no passo a passo (so na transicao "em andamento" -> "terminada", para nao
            // regravar o arquivo a cada clique depois do fim).
            if (!jaTerminada && simulador.todasTerminadas())
                mensagem = exportarGantt(simulador,
                                         "Simulacao concluida no tick " + to_string(simulador.getTickAtual()) + ". ");
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

        // Seleciona o algoritmo de escalonamento (lista vinda do registro).
        ImGui::SetNextItemWidth(100);

        if (ImGui::BeginCombo("Algoritmo", nomesAlg[algoritmo].c_str()))
        {
            for (int i = 0; i < static_cast<int>(nomesAlg.size()); i++)
                if (ImGui::Selectable(nomesAlg[i].c_str(), i == algoritmo))
                {
                    algoritmo = i;
                    configuracaoAlterada = true;
                }
            ImGui::EndCombo();
        }

        ImGui::SameLine();

        // Quantidade de CPUs configuravel pelo usuario de 1 ate N (requisito geral 2). N e o maior
        // entre o valor do arquivo e LIMITE_CPUS: o arquivo e so o valor inicial, nao um teto.
        ImGui::SetNextItemWidth(100);

        constexpr int LIMITE_CPUS = 64;
        int maxCPUs = max(config.qtdeCpus, LIMITE_CPUS);

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
            simulador.definirAlgoritmo(nomesAlg[algoritmo]);
            simulador.definirQuantidadeCPUs(quantidadeCPUs);
            simulador.definirQuantum(quantum);
            mensagem = "Configuracao atualizada no tick " + to_string(simulador.getTickAtual()) +
                       ". O historico anterior foi preservado.";
        }

        // Carregar outro arquivo de configuracao sem fechar o programa (requisito 3.3.4):
        // aceita qualquer nome e qualquer local do sistema de arquivos. Se o arquivo tiver
        // erros, a simulacao atual e mantida e os erros sao mostrados.
        ImGui::Separator();
        ImGui::SetNextItemWidth(420);
        ImGui::InputText("Arquivo de configuracao", caminhoNovo, sizeof(caminhoNovo));
        ImGui::SameLine();
        if (ImGui::Button("Carregar"))
        {
            Configuracao nova;
            const bool carregou = carregarConfiguracao(caminhoNovo, nova);
            string texto;
            for (const string &erroCfg : nova.erros)
                texto += "Erro: " + erroCfg + "\n";
            for (const string &aviso : nova.avisos)
                texto += "Aviso: " + aviso + "\n";
            if (carregou)
            {
                config = nova;
                simulador.reconfigurar(config.qtdeCpus, config.quantum, config.algoritmo, config.tarefas);
                algoritmo = indiceDoAlgoritmo(config.algoritmo);
                quantidadeCPUs = simulador.getQuantidadeCPUs();
                quantum = simulador.getQuantum();
                mensagem = "Configuracao carregada de " + string(caminhoNovo) + ".\n" + texto;
            }
            else
                mensagem = "Nao foi possivel carregar a configuracao (a simulacao atual foi mantida):\n" + texto;
        }

        // Encerra a janela de controles.
        ImGui::End();

        // Desenha o gráfico e a tabela com os dados atualizados do simulador.
        desenharGantt(simulador);
        desenharTarefas(simulador);
        desenharEditorTarefas(simulador, mensagem);
        desenharCPUs(simulador);
        desenharEstadoSistema(simulador);
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