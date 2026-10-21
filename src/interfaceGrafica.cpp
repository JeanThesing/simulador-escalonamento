#include "header/interfaceGrafica.hpp"

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
    // Obtém o histórico de execução e a lista de tarefas.
    const auto &historico = simulador.getHistoricoGantt();
    const auto &tarefas = simulador.getTarefas();

    // Cria a janela do gráfico.
    ImGui::Begin("Grafico de Gantt");

    // Exibe o tick atual da simulação.
    ImGui::Text("Tick atual: %d", simulador.getTickAtual());
    ImGui::Separator();

    // Cria uma área com rolagem horizontal.
    ImGui::BeginChild(
        "Gantt",
        ImVec2(0, 0),
        ImGuiChildFlags_None,
        ImGuiWindowFlags_HorizontalScrollbar);

    // Obtém as ferramentas de desenho e a posição inicial.
    ImDrawList *draw = ImGui::GetWindowDrawList();
    ImVec2 inicio = ImGui::GetCursorScreenPos();

    // Define as dimensões dos elementos do gráfico.
    const float larguraRotulo = 70.0f;
    const float larguraTick = 90.0f;
    const float alturaLinha = 36.0f;

    // Calcula o espaço necessário para o gráfico.
    float largura = larguraRotulo + historico.size() * larguraTick;
    float altura = 30.0f + tarefas.size() * alturaLinha;

    // Reserva espaço para permitir a exibição e rolagem do gráfico.
    ImGui::Dummy(ImVec2(largura, altura));

    // Desenha o cabeçalho com os números dos ticks.
    for (size_t t = 0; t < historico.size(); ++t)
    {
        char texto[16];
        std::snprintf(texto, sizeof(texto), "%zu", t);

        draw->AddText(
            ImVec2(
                inicio.x + larguraRotulo + t * larguraTick + 8,
                inicio.y + 5),
            IM_COL32(220, 220, 220, 255),
            texto);
    }

    // Desenha uma linha para cada tarefa.
    for (size_t i = 0; i < tarefas.size(); ++i)
    {
        const Tarefa &tarefa = tarefas[i];
        float y = inicio.y + 30 + i * alturaLinha;

        // Exibe o identificador da tarefa.
        char nomeTarefa[24];
        std::snprintf(nomeTarefa, sizeof(nomeTarefa), "T%d", tarefa.getId());

        draw->AddText(
            ImVec2(inicio.x, y + 8),
            IM_COL32(230, 230, 230, 255),
            nomeTarefa);

        // Verifica em qual CPU a tarefa executou em cada tick.
        for (size_t t = 0; t < historico.size(); ++t)
        {
            // -1 indica que a tarefa não executou nesse tick.
            int cpuExecutora = -1;

            // Procura a tarefa entre as CPUs do tick atual.
            for (size_t c = 0; c < historico[t].size(); ++c)
            {
                if (historico[t][c] == tarefa.getId())
                {
                    cpuExecutora = static_cast<int>(c);
                    break;
                }
            }

            // Calcula a posição horizontal da célula.
            float x = inicio.x + larguraRotulo + t * larguraTick;

            // Define a cor conforme a execução da tarefa.
            ImU32 cor = cpuExecutora >= 0
                            ? corTarefa(tarefa.getId())
                            : IM_COL32(65, 70, 80, 255);

            // Desenha a célula correspondente ao tick.
            draw->AddRectFilled(
                ImVec2(x + 1, y + 1),
                ImVec2(x + larguraTick - 2, y + alturaLinha - 2),
                cor,
                3.0f);

            // Exibe a CPU responsável quando houve execução.
            if (cpuExecutora >= 0)
            {
                char nomeCPU[16];
                std::snprintf(nomeCPU, sizeof(nomeCPU), "CPU %d", cpuExecutora);

                draw->AddText(
                    ImVec2(x + 8, y + 9),
                    IM_COL32(255, 255, 255, 255),
                    nomeCPU);
            }
        }
    }

    // Finaliza a área interna e a janela do gráfico.
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