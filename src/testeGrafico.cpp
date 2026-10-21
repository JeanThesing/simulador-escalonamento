// Raylib: cria a janela do programa, controla o desenho e gerencia a execução gráfica.
#include "raylib.h"

// rlImGui: conecta a Raylib ao Dear ImGui, permitindo desenhar os componentes da interface.
#include "rlImGui.h"

// Dear ImGui: biblioteca utilizada para criar janelas, botões, tabelas e outros elementos da interface.
#include "imgui.h"

// Inclui a classe Simulador para acessar os dados reais da simulação.
#include "header/simulador.hpp"

#include <string>
#include <cstdio>

// Retorna a cor que será utilizada para desenhar uma tarefa no gráfico de Gantt.
// Cada ID de tarefa recebe uma cor. O ID -1 representa uma CPU ociosa.
ImU32 corTarefa(int id)
{
    // IM_COL32 recebe os valores de vermelho, verde, azul e transparência.
    // Os valores RGB vão de 0 a 255.
    const ImU32 cores[] = {
        IM_COL32(230, 90, 80, 255),
        IM_COL32(70, 170, 120, 255),
        IM_COL32(80, 140, 230, 255),
        IM_COL32(190, 120, 220, 255),
        IM_COL32(230, 190, 70, 255)};

    // Se não houver tarefa, retorna cinza.
    // Caso contrário, escolhe uma das cinco cores usando o ID da tarefa.
    return id < 0 ? IM_COL32(65, 70, 80, 255)
                  : cores[id % 5];
}

// Converte o estado interno da tarefa em um texto que possa ser exibido na interface.
const char *nomeEstado(EstadoTarefa estado)
{
    switch (estado)
    {
    case EstadoTarefa::NOVA:
        return "Nova";

    case EstadoTarefa::PRONTA:
        return "Pronta";

    case EstadoTarefa::EXECUTANDO:
        return "Executando";

    case EstadoTarefa::SUSPENSA:
        return "Suspensa";

    case EstadoTarefa::AGUARDANDO:
        return "Aguardando";

    case EstadoTarefa::TERMINADA:
        return "Terminada";

    default:
        return "Desconhecido";
    }
}

// Desenha o gráfico de Gantt usando o histórico real do simulador.
// Cada linha representa uma CPU e cada coluna representa um tick.
void desenharGantt(const Simulador &simulador)
{
    // Obtém referências aos dados do simulador.
    // O uso de const auto& evita copiar os vetores e impede alterações nesses dados.
    const auto &historico = simulador.getHistoricoGantt();
    const auto &cpus = simulador.getCPUs();

    // Begin() abre uma janela do ImGui com o título informado.
    // Tudo o que for desenhado até ImGui::End() pertence a essa janela.
    ImGui::Begin("Grafico de Gantt");

    // Text() exibe texto simples dentro da janela.
    ImGui::Text("Tick atual: %d", simulador.getTickAtual());

    // Separator() desenha uma linha horizontal para separar visualmente os elementos.
    ImGui::Separator();

    // BeginChild() cria uma área interna à janela.
    // Ela pode ter sua própria barra de rolagem, útil quando o gráfico fica muito largo.
    ImGui::BeginChild(
        "Gantt",                             // Identificador da área interna.
        ImVec2(0, 0),                        // Largura e altura; zero utiliza o espaço disponível.
        ImGuiChildFlags_None,                // Não aplica opções adicionais de borda.
        ImGuiWindowFlags_HorizontalScrollbar // Permite rolagem horizontal.
    );

    // ImDrawList permite desenhar formas e textos manualmente.
    // É o que usamos para construir as células do gráfico de Gantt.
    ImDrawList *draw = ImGui::GetWindowDrawList();

    // GetCursorScreenPos() retorna a posição, na tela, onde começa a área de conteúdo.
    // Essa posição será a referência para desenhar todos os elementos do gráfico.
    ImVec2 inicio = ImGui::GetCursorScreenPos();

    // Define as dimensões de cada parte do gráfico, em pixels.
    const float larguraRotulo = 60.0f; // Espaço reservado aos nomes das CPUs.
    const float larguraTick = 52.0f;   // Largura de cada coluna do gráfico.
    const float alturaLinha = 36.0f;   // Altura de cada linha de CPU.

    // Calcula o espaço total necessário para desenhar o gráfico.
    float largura = larguraRotulo + historico.size() * larguraTick;
    float altura = 30.0f + cpus.size() * alturaLinha;

    // Dummy() reserva espaço dentro da área do ImGui.
    // Como desenhamos usando ImDrawList, o ImGui precisa saber quanto espaço ocupar.
    ImGui::Dummy(ImVec2(largura, altura));

    // Desenha o cabeçalho com a numeração dos ticks.
    for (size_t t = 0; t < historico.size(); ++t)
    {
        char texto[16];

        // Formata o número do tick como texto.
        // %zu é o especificador de formato utilizado para size_t.
        std::snprintf(texto, sizeof(texto), "%zu", t + 1);

        // AddText() desenha texto diretamente na posição indicada.
        // A posição é calculada a partir do início do gráfico e da coluna atual.
        draw->AddText(
            ImVec2(
                inicio.x + larguraRotulo + t * larguraTick + 8,
                inicio.y + 5),
            IM_COL32(220, 220, 220, 255),
            texto);
    }

    // Percorre todas as CPUs para desenhar uma linha para cada uma.
    for (size_t c = 0; c < cpus.size(); ++c)
    {
        // Calcula a posição vertical da linha da CPU atual.
        float y = inicio.y + 30 + c * alturaLinha;

        char nome[24];

        // Cria o rótulo da CPU, como "CPU 0" ou "CPU 1".
        std::snprintf(nome, sizeof(nome), "CPU %d", cpus[c].getId());

        draw->AddText(
            ImVec2(inicio.x, y + 8),
            IM_COL32(230, 230, 230, 255),
            nome);

        // Percorre o histórico para desenhar uma célula para cada tick.
        for (size_t t = 0; t < historico.size(); ++t)
        {
            // Verifica se existe uma informação para esta CPU neste tick.
            if (c >= historico[t].size())
                continue;

            // Obtém o ID da tarefa executada pela CPU neste tick.
            // O valor -1 significa que a CPU estava ociosa.
            int id = historico[t][c];

            // Calcula a posição horizontal da célula.
            float x = inicio.x + larguraRotulo + t * larguraTick;

            // AddRectFilled() desenha um retângulo preenchido.
            // As duas posições definem seus cantos superior esquerdo e inferior direito.
            draw->AddRectFilled(
                ImVec2(x + 1, y + 1),
                ImVec2(x + larguraTick - 2, y + alturaLinha - 2),
                corTarefa(id),
                3.0f // Raio dos cantos arredondados.
            );

            // Quando existe uma tarefa, escreve seu ID dentro da célula.
            if (id >= 0)
            {
                char nomeTarefa[16];

                std::snprintf(
                    nomeTarefa,
                    sizeof(nomeTarefa),
                    "T%d",
                    id);

                draw->AddText(
                    ImVec2(x + 8, y + 9),
                    IM_COL32(255, 255, 255, 255),
                    nomeTarefa);
            }
        }
    }

    // Encerra a área interna e, em seguida, a janela do gráfico.
    // Todo Begin() deve ter seu End() correspondente.
    ImGui::EndChild();
    ImGui::End();
}

// Desenha uma tabela com informações atuais das tarefas.
void desenharTarefas(const Simulador &simulador)
{
    ImGui::Begin("Tarefas");

    // BeginTable() cria uma tabela com quatro colunas.
    // As opções ativam bordas, alternância de cor das linhas e redimensionamento das colunas.
    if (ImGui::BeginTable(
            "TabelaTarefas",
            4,
            ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_Resizable))
    {
        // Define os nomes das colunas.
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Estado");
        ImGui::TableSetupColumn("Duracao");
        ImGui::TableSetupColumn("Restante");

        // Desenha o cabeçalho da tabela.
        ImGui::TableHeadersRow();

        // Percorre as tarefas reais armazenadas no simulador.
        for (const auto &tarefa : simulador.getTarefas())
        {
            // Cria uma nova linha para a tarefa atual.
            ImGui::TableNextRow();

            // TableNextColumn() avança para a próxima coluna da linha.
            ImGui::TableNextColumn();
            ImGui::Text("%d", tarefa.getId());

            ImGui::TableNextColumn();
            ImGui::Text("%s", nomeEstado(tarefa.getEstado()));

            ImGui::TableNextColumn();
            ImGui::Text("%d", tarefa.getDuracao());

            ImGui::TableNextColumn();
            ImGui::Text("%d", tarefa.getTempoRestante());
        }

        // Encerra a tabela.
        ImGui::EndTable();
    }

    // Encerra a janela das tarefas.
    ImGui::End();
}

int main()
{
    // Cria a janela da aplicação com largura 1200 e altura 750 pixels.
    InitWindow(1200, 750, "Simulador de Escalonamento");

    // Define a taxa de atualização da janela: até 60 quadros por segundo.
    SetTargetFPS(60);

    // Inicializa a integração entre Raylib e Dear ImGui.
    // O parâmetro true ativa o suporte aos recursos de fonte padrão do rlImGui.
    rlImGuiSetup(true);

    // Cria o simulador com duas CPUs, quantum 2 e algoritmo RM.
    Simulador simulador(2, 2, "RM");

    // Inicializa os componentes internos da simulação.
    simulador.criaCPUS();
    simulador.criaTarefas();
    simulador.criaEscalonador();

    // Salva o estado inicial para que o histórico tenha um ponto de partida.
    simulador.salvaHistoricoTick();

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
            simulador.retrocedeTick();

        // SameLine() coloca o próximo componente na mesma linha do anterior.
        ImGui::SameLine();

        if (ImGui::Button("Proximo tick"))
            simulador.avancaTick();

        ImGui::SameLine();

        if (ImGui::Button("Resetar"))
            simulador.reiniciar();

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
