#include "header/interfaceGrafica.hpp"

// Retorna a cor da tarefa definida no arquivo de configuração (RGB hexadecimal).
ImU32 corTarefa(const Tarefa &tarefa)
{
    // Converte a cor hexadecimal para um número inteiro.
    unsigned int rgb = std::strtoul(tarefa.getCor().c_str(), nullptr, 16);

    // Extrai os componentes vermelho, verde e azul.
    int vermelho = (rgb >> 16) & 0xFF;
    int verde = (rgb >> 8) & 0xFF;
    int azul = rgb & 0xFF;

    return IM_COL32(vermelho, verde, azul, 255);
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
// ma linha por tarefa
void desenharGantt(const Simulador &simulador)
{
    // Obtém o histórico de execução, lista de cpus e a lista de tarefas.
    const auto &historico = simulador.getHistoricoGantt();
    const auto &tarefas = simulador.getTarefas();
    const auto &cpus = simulador.getCPUs();

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
    float altura = 30.0f + (tarefas.size() + cpus.size()) * alturaLinha;

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
        // A tarefa com menor ID é desenhada na linha mais baixa.
        float y = inicio.y + 30 + (tarefas.size() - 1 - i) * alturaLinha; // menor ID embaixo

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
            ImU32 cor = cpuExecutora >= 0 ? corTarefa(tarefa) : IM_COL32(65, 70, 80, 255);

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
        // Desenha os eventos da tarefa no gráfico de Gantt.
        for (const auto &evento : simulador.getEventos())
        {
            // Ignora eventos de outras tarefas.
            if (evento.idTarefa != tarefa.getId())
                continue;

            // Calcula a posição horizontal do evento no gráfico.
            float x = inicio.x + larguraRotulo + evento.tick * larguraTick;

            switch (evento.tipo)
            {
            case TipoEvento::CHEGADA:
                // Triângulo verde apontando para baixo.
                draw->AddTriangleFilled(
                    ImVec2(x - 5, y),
                    ImVec2(x + 5, y),
                    ImVec2(x, y + 10),
                    IM_COL32(80, 220, 120, 255));
                break;

            case TipoEvento::TERMINO:
                // Triângulo azul apontando para cima.
                draw->AddTriangleFilled(
                    ImVec2(x - 5, y + alturaLinha),
                    ImVec2(x + 5, y + alturaLinha),
                    ImVec2(x, y + alturaLinha - 10),
                    IM_COL32(80, 160, 255, 255));
                break;

            case TipoEvento::PRAZO_PERDIDO:
                // X vermelho indicando que a tarefa perdeu o prazo.
                draw->AddLine(
                    ImVec2(x - 6, y + 4),
                    ImVec2(x + 6, y + alturaLinha - 4),
                    IM_COL32(255, 60, 60, 255), 2.0f);

                draw->AddLine(
                    ImVec2(x - 6, y + alturaLinha - 4),
                    ImVec2(x + 6, y + 4),
                    IM_COL32(255, 60, 60, 255), 2.0f);
                break;
            }
        }
    }

    // Uma linha por CPU: marca os ticks em que ela ficou desligada (sem tarefa)
    for (size_t c = 0; c < cpus.size(); ++c)
    {
        float y = inicio.y + 30 + (tarefas.size() + c) * alturaLinha;

        char nomeCPU[16];
        std::snprintf(nomeCPU, sizeof(nomeCPU), "CPU %d", cpus[c].getId());
        draw->AddText(ImVec2(inicio.x, y + 8), IM_COL32(230, 230, 230, 255), nomeCPU);

        for (size_t t = 0; t < historico.size(); ++t)
        {
            // -1 no historico = CPU sem tarefa neste tick = desligada
            if (c < historico[t].size() && historico[t][c] == -1)
            {
                float x = inicio.x + larguraRotulo + t * larguraTick;
                draw->AddRectFilled(ImVec2(x + 1, y + 1), ImVec2(x + larguraTick - 2, y + alturaLinha - 2),
                                    IM_COL32(40, 40, 45, 255), 3.0f);
                draw->AddText(ImVec2(x + 8, y + 9), IM_COL32(150, 150, 150, 255), "off");
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