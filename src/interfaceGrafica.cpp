#include "header/interfaceGrafica.hpp"
#include "header/exportadorGantt.hpp"

// Cor da célula em que a tarefa não executou naquele tick (mesma no Gantt e na legenda).
static const ImU32 COR_SEM_EXECUCAO = IM_COL32(24, 27, 34, 255);

// Retorna a cor que será utilizada para desenhar uma tarefa no gráfico de Gantt.
// Usa a cor informada no arquivo de configuração (RRGGBB), a mesma da imagem exportada.
ImU32 corTarefa(const Tarefa &tarefa)
{
    int r = 128, g = 128, b = 128; // cinza caso a cor seja inválida
    corParaRGB(tarefa.getCor(), r, g, b);

    // IM_COL32 recebe os valores de vermelho, verde, azul e transparência (0 a 255).
    return IM_COL32(r, g, b, 255);
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

// Desenha a tabela com o estado de cada CPU (ligada/desligada, tarefa atual e tempo desligada).
void desenharCPUs(const Simulador &simulador)
{
    ImGui::Begin("CPUs");

    if (ImGui::BeginTable(
            "TabelaCPUs",
            4,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("CPU");
        ImGui::TableSetupColumn("Estado");
        ImGui::TableSetupColumn("Tarefa");
        ImGui::TableSetupColumn("Tempo desligada");
        ImGui::TableHeadersRow();

        for (const CPU &cpu : simulador.getCPUs())
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", cpu.getId());

            ImGui::TableNextColumn();
            ImGui::Text("%s", cpu.getLigado() ? "Ligada" : "Desligada");

            ImGui::TableNextColumn();
            if (cpu.getTarefaAtual())
                ImGui::Text("T%d", cpu.getTarefaAtual()->getId());
            else
                ImGui::Text("-");

            // Total acumulado de ticks em que a CPU ficou sem tarefa (requisito 1.2).
            ImGui::TableNextColumn();
            ImGui::Text("%d ticks", cpu.getTempoDesligado());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

// Desenha o gráfico de Gantt usando o histórico real do simulador.
// ma linha por tarefa
void desenharGantt(const Simulador &simulador)
{
    // Obtém o histórico de execução, lista de cpus e a lista de tarefas.
    const auto &historico = simulador.getHistoricoGantt();
    const auto &historicoEstados = simulador.getHistorico();
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
    size_t maxCPUsHistorico = cpus.size();
    for (const EstadoSistema &estado : historicoEstados)
        maxCPUsHistorico = std::max(maxCPUsHistorico, estado.cpus.size());
    float altura = 30.0f + (tarefas.size() + maxCPUsHistorico) * alturaLinha;

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

            // Estado no início deste tick, preservado pelo histórico reversível.
            EstadoTarefa estadoTick = EstadoTarefa::NOVA;
            if (t < historicoEstados.size())
            {
                for (const Tarefa &snapshot : historicoEstados[t].tarefas)
                    if (snapshot.getId() == tarefa.getId())
                    {
                        estadoTick = snapshot.getEstado();
                        break;
                    }
            }
            const bool suspensa = cpuExecutora < 0 && estadoTick == EstadoTarefa::SUSPENSA;
            ImU32 cor = cpuExecutora >= 0 ? corTarefa(tarefa) : (suspensa ? IM_COL32(0, 0, 0, 255) : COR_SEM_EXECUCAO);

            // Pronta/inativa fica sem preenchimento; suspensa fica preta e pontilhada.
            draw->AddRectFilled(ImVec2(x + 1, y + 1), ImVec2(x + larguraTick - 2, y + alturaLinha - 2), cor, 3.0f);
            draw->AddRect(ImVec2(x + 1, y + 1), ImVec2(x + larguraTick - 2, y + alturaLinha - 2), IM_COL32(70, 75, 85, 255), 3.0f);
            if (suspensa)
            {
                for (float px = x + 5; px < x + larguraTick - 4; px += 8)
                    for (float py = y + 5; py < y + alturaLinha - 4; py += 8)
                        draw->AddCircleFilled(ImVec2(px, py), 1.5f, IM_COL32(190, 190, 190, 255));
            }

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

            case TipoEvento::SORTEIO:
            {
                // Losango laranja no meio da celula: a tarefa foi escolhida por sorteio
                // (empate total nos criterios de desempate) e comecou a executar neste tick.
                float cy = y + alturaLinha / 2;
                ImVec2 losango[4] = {
                    ImVec2(x, cy - 8), ImVec2(x + 8, cy),
                    ImVec2(x, cy + 8), ImVec2(x - 8, cy)};
                draw->AddConvexPolyFilled(losango, 4, IM_COL32(255, 159, 28, 255));
                draw->AddPolyline(losango, 4, IM_COL32(20, 20, 20, 255), ImDrawFlags_Closed, 1.5f);
                break;
            }
            }
        }
    }

    // Uma linha por CPU: marca os períodos em que ela ficou desligada (sem tarefa).
    for (size_t c = 0; c < maxCPUsHistorico; ++c)
    {
        float y = inicio.y + 30 + (tarefas.size() + c) * alturaLinha;

        char nomeCPU[16];
        std::snprintf(nomeCPU, sizeof(nomeCPU), "CPU %zu", c);
        draw->AddText(ImVec2(inicio.x, y + 8), IM_COL32(230, 230, 230, 255), nomeCPU);

        // Junta ticks seguidos desligados em uma única faixa, mostrando a duração do período.
        size_t t = 0;
        while (t < historico.size())
        {
            // -1 no histórico = CPU sem tarefa neste tick = desligada
            bool cpuExistiaNoTick = t < historicoEstados.size() && c < historicoEstados[t].cpus.size();
            bool desligada = cpuExistiaNoTick && c < historico[t].size() && historico[t][c] == -1;

            if (!desligada)
            {
                t++;
                continue;
            }

            // Avança até o fim do período desligado.
            size_t inicioFaixa = t;

            while (t < historico.size() &&
                   t < historicoEstados.size() &&
                   c < historicoEstados[t].cpus.size() &&
                   c < historico[t].size() &&
                   historico[t][c] == -1)
            {
                t++;
            }

            float x1 = inicio.x + larguraRotulo + inicioFaixa * larguraTick;
            float x2 = inicio.x + larguraRotulo + t * larguraTick;

            draw->AddRectFilled(
                ImVec2(x1 + 1, y + 1),
                ImVec2(x2 - 2, y + alturaLinha - 2),
                IM_COL32(40, 40, 45, 255), 3.0f);

            char texto[32];
            std::snprintf(texto, sizeof(texto), "off (%zu)", t - inicioFaixa);

            draw->AddText(
                ImVec2(x1 + 8, y + 9),
                IM_COL32(150, 150, 150, 255),
                texto);
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

    // A tabela apresenta o estado atual e os parâmetros temporais de cada tarefa.
    if (ImGui::BeginTable(
            "TabelaTarefas",
            7,
            ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Estado");
        ImGui::TableSetupColumn("Ingresso");
        ImGui::TableSetupColumn("Duracao");
        ImGui::TableSetupColumn("Periodo");
        ImGui::TableSetupColumn("Prazo");
        ImGui::TableSetupColumn("Restante");

        ImGui::TableHeadersRow();

        for (const auto &tarefa : simulador.getTarefas())
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", tarefa.getId());

            ImGui::TableNextColumn();
            ImGui::Text("%s", nomeEstado(tarefa.getEstado()));

            ImGui::TableNextColumn();
            ImGui::Text("%d", tarefa.getIngresso());

            ImGui::TableNextColumn();
            ImGui::Text("%d", tarefa.getDuracao());

            ImGui::TableNextColumn();
            ImGui::Text("%d", tarefa.getPeriodo());

            ImGui::TableNextColumn();
            ImGui::Text("%d", tarefa.getPrazo());

            ImGui::TableNextColumn();
            ImGui::Text("%d", tarefa.getTempoRestante());
        }

        // Encerra a tabela.
        ImGui::EndTable();
    }

    // Encerra a janela das tarefas.
    ImGui::End();
}

// Desenha a legenda dos elementos do gráfico de Gantt mostrado na janela.
// Lista as cores das células e os marcadores de evento (chegada, término, prazo perdido,
// sorteio); a imagem exportada tem a sua própria legenda com os mesmos elementos.
void desenharLegenda(const Simulador &simulador)
{
    ImGui::SetNextWindowPos(ImVec2(900, 40), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 260), ImGuiCond_FirstUseEver);
    ImGui::Begin("Legenda");

    ImDrawList *draw = ImGui::GetWindowDrawList();

    // Desenha um quadradinho de amostra seguido do texto explicativo.
    auto itemLegenda = [&](ImU32 cor, const char *texto)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        draw->AddRectFilled(ImVec2(p.x, p.y + 2), ImVec2(p.x + 26, p.y + 16), cor, 3.0f);

        // Reserva o espaço da amostra para o texto ficar ao lado dela.
        ImGui::Dummy(ImVec2(26, 18));
        ImGui::SameLine();
        ImGui::TextUnformatted(texto);
    };

    // Uma entrada por tarefa, na ordem do ID, com a mesma cor usada nas células do Gantt.
    for (const Tarefa &tarefa : simulador.getTarefas())
    {
        char texto[48];
        std::snprintf(texto, sizeof(texto), "Tarefa %d executando", tarefa.getId());
        itemLegenda(corTarefa(tarefa), texto);
    }

    itemLegenda(COR_SEM_EXECUCAO, "Sem CPU neste tick (pronta ou inativa)");
    itemLegenda(IM_COL32(40, 40, 45, 255), "CPU desligada (off (N) = N ticks seguidos)");
    itemLegenda(IM_COL32(0, 0, 0, 255), "Suspensa (preto pontilhado)");

    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.31f, 0.86f, 0.47f, 1), "Triangulo verde (para baixo): chegada");
    ImGui::TextColored(ImVec4(0.31f, 0.63f, 1.0f, 1), "Triangulo azul (para cima): termino");
    ImGui::TextColored(ImVec4(1.0f, 0.24f, 0.24f, 1), "X vermelho: prazo perdido");
    ImGui::TextColored(ImVec4(1.0f, 0.62f, 0.11f, 1), "Losango laranja: sorteio (desempate)");

    ImGui::TextWrapped("O texto \"CPU n\" dentro da celula indica em qual processador a tarefa executou.");

    ImGui::End();
}

void desenharEditorTarefas(Simulador &simulador, std::string &mensagem)
{
    static int indiceTarefa = 0;
    static int estadoSelecionado = 1;
    static int tempoRestante = 1;
    static int ingresso = 0, duracao = 1, periodo = 1, prazo = 1;
    static char corHex[8] = "000000";
    static int ultimaTarefa = -1;
    static int ultimoTick = -1;
    static std::string resultado; // resultado da última tentativa de aplicar, mostrado no próprio editor
    static bool resultadoErro = false;
    const auto &tarefas = simulador.getTarefas();
    const char *estados[] = {"Nova", "Pronta", "Executando", "Suspensa", "Aguardando", "Terminada"};
    if (ultimoTick != simulador.getTickAtual())
    {
        ultimaTarefa = -1;
        ultimoTick = simulador.getTickAtual();
    }

    ImGui::SetNextWindowSize(ImVec2(430, 470), ImGuiCond_FirstUseEver);
    ImGui::Begin("Editar tarefa");
    if (tarefas.empty())
    {
        ImGui::TextUnformatted("Nenhuma tarefa carregada.");
        ImGui::End();
        return;
    }
    if (indiceTarefa < 0 || indiceTarefa >= static_cast<int>(tarefas.size()))
        indiceTarefa = 0;
    std::vector<std::string> nomes;
    std::vector<const char *> opcoes;
    nomes.reserve(tarefas.size());
    opcoes.reserve(tarefas.size());
    for (const auto &t : tarefas)
        nomes.push_back("Tarefa " + std::to_string(t.getId()));
    for (const auto &n : nomes)
        opcoes.push_back(n.c_str());

    // ---------- Tarefa selecionada ----------
    ImGui::SeparatorText("Tarefa selecionada");
    int indiceAnterior = indiceTarefa;
    ImGui::SetNextItemWidth(160.0f);
    ImGui::Combo("Tarefa", &indiceTarefa, opcoes.data(), static_cast<int>(opcoes.size()));

    const Tarefa &t = tarefas[static_cast<size_t>(indiceTarefa)];
    if (ultimaTarefa != indiceTarefa || indiceAnterior != indiceTarefa)
    {
        // Carrega os valores atuais da tarefa nos campos do editor.
        estadoSelecionado = static_cast<int>(t.getEstado());
        tempoRestante = t.getTempoRestante();
        ingresso = t.getIngresso();
        duracao = t.getDuracao();
        periodo = t.getPeriodo();
        prazo = t.getPrazo();
        std::snprintf(corHex, sizeof(corHex), "%s", t.getCor().c_str());
        if (indiceAnterior != indiceTarefa)
            resultado.clear(); // o resultado anterior era de outra tarefa
        ultimaTarefa = indiceTarefa;
    }
    ImGui::Text("ID: %d | CPU: %d | Estado: %s", t.getId(), t.getIdCPU(), nomeEstado(t.getEstado()));

    // ---------- Características temporais ----------
    ImGui::SeparatorText("Caracteristicas temporais");
    ImGui::SetNextItemWidth(160.0f);
    ImGui::InputInt("Ingresso", &ingresso);
    ImGui::SetNextItemWidth(160.0f);
    ImGui::InputInt("Duracao", &duracao);
    ImGui::SetNextItemWidth(160.0f);
    ImGui::InputInt("Periodo", &periodo);
    ImGui::SetNextItemWidth(160.0f);
    ImGui::InputInt("Prazo", &prazo);


    // PARA evitar que o usuário digite valores negativos ou menores que 1, para valores q n  fazem sentido
    if (ingresso < 0)
        ingresso = 0;
    if (duracao < 1)
        duracao = 1;
    if (periodo < 1)
        periodo = 1;
    if (prazo < 1)
        prazo = 1;

    // ---------- Estado atual ----------
    ImGui::SeparatorText("Estado atual");
    ImGui::SetNextItemWidth(160.0f);
    ImGui::Combo("Novo estado", &estadoSelecionado, estados, IM_ARRAYSIZE(estados));
    ImGui::SetNextItemWidth(160.0f);
    ImGui::InputInt("Tempo restante", &tempoRestante);
    // Impede que numeros negativos permaneçam no campo; o limite superior (duração) e as
    // demais incompatibilidades são validados ao aplicar, com uma justificativa para o usuário.
    if (tempoRestante < 0)
        tempoRestante = 0;

    // ---------- Cor da tarefa ----------
    ImGui::SeparatorText("Cor da tarefa");
    ImGui::SetNextItemWidth(160.0f);
    ImGui::InputText("RRGGBB", corHex, sizeof(corHex),
                     ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
    // Amostra da cor digitada (cinza enquanto o valor for inválido).
    {
        int r = 128, g = 128, b = 128;
        corParaRGB(corHex, r, g, b);
        ImGui::SameLine();
        ImGui::ColorButton("##amostraCor", ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f),
                           ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
    }

    ImGui::Spacing();
    ImGui::TextWrapped("Alteracoes sao validadas e registradas no historico. "
                       "Campos incompativeis com o estado atual sao rejeitados com uma justificativa.");
    ImGui::Spacing();

    if (ImGui::Button("Aplicar alteracoes"))
    {
        std::string erro;
        const int idTarefa = t.getId();
        EdicaoTarefa edicao{static_cast<EstadoTarefa>(estadoSelecionado), tempoRestante,
                            ingresso, duracao, periodo, prazo, corHex};
        if (simulador.editarTarefa(idTarefa, edicao, erro))
        {
            resultado = "Alteracao aplicada a tarefa " + std::to_string(idTarefa) + ".";
            resultadoErro = false;
            ultimaTarefa = -1; // recarrega os valores atuais após aplicar a edição
        }
        else
        {
            resultado = "Nao foi possivel alterar a tarefa: " + erro;
            resultadoErro = true;
        }
        mensagem = resultado; // também aparece no painel de controle
    }

    // Resultado da última tentativa, junto do botão (verde = aplicada, vermelho = rejeitada).
    if (!resultado.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, resultadoErro ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f)
                                                           : ImVec4(0.35f, 0.85f, 0.45f, 1.0f));
        ImGui::TextWrapped("%s", resultado.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::End();
}