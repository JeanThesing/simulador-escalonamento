#include "header/exportadorGantt.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <vector>

/*
 * Exportacao do grafico de Gantt para SVG (requisito 2.4).
 *
 * Por que SVG: e imagem escrita como texto, entao nao precisa de biblioteca extra
 * (requisito geral 5) e e vetorial, sem limite de tamanho para simulacoes longas.
 * Usa so coordenadas inteiras, para o arquivo nao depender da configuracao regional
 * do computador (virgula x ponto decimal). Nenhum texto digitado pelo usuario vai para
 * o SVG (so numeros e o nome do algoritmo), por isso nao e preciso escapar caracteres XML.
 */

namespace
{
    // Geometria (pixels)
    constexpr int MARGEM_ESQ = 90;      // rotulos das tarefas (T1, T2...)
    constexpr int MARGEM_TOPO = 56;     // titulo
    constexpr int LARGURA_TICK = 36;    // largura de 1 tick
    constexpr int ALTURA_LINHA = 48;    // faixa de cada tarefa
    constexpr int FOLGA_BARRA = 14;     // espaco acima/abaixo da barra, onde ficam os marcadores
    constexpr int LARGURA_MINIMA = 760; // garante espaco para a legenda em simulacoes curtas
    constexpr int LINHA_LEGENDA = 28;
    constexpr int AMOSTRA = 26; // largura do desenho de amostra na legenda

    // Estados de uma tarefa em um tick (os valores >= 0 sao o id da CPU que executou)
    constexpr int INATIVA = -2; // ainda nao chegou ou ja terminou a ativacao
    constexpr int PRONTA = -1;  // na fila de prontas, esperando CPU

    // ---------- Elementos graficos (usados na grade e na legenda) ----------

    // Barra de execucao: cor da tarefa (arquivo de configuracao) e rotulo com a CPU usada.
    // O texto fica preto em cores claras e branco em escuras (luminancia), para sempre ler.
    void desenharBarra(std::ostream &o, int x, int y, int w, int h, const Tarefa &tarefa, const std::string &rotulo)
    {
        int r = 128, g = 128, b = 128; // cinza se a cor for invalida
        corParaRGB(tarefa.getCor(), r, g, b);
        char cor[8];
        std::snprintf(cor, sizeof(cor), "#%02X%02X%02X", r, g, b);

        o << "<rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << w << "\" height=\"" << h << "\" fill=\"" << cor
          << "\" stroke=\"#222\"/>\n";
        if (!rotulo.empty())
            o << "<text x=\"" << x + w / 2 << "\" y=\"" << y + h / 2 + 4
              << "\" text-anchor=\"middle\" font-size=\"11\" font-weight=\"bold\" fill=\""
              << (0.299 * r + 0.587 * g + 0.114 * b > 150 ? "#000" : "#FFF") << "\">" << rotulo << "</text>\n";
    }

    // Desenha um losango laranja para indicar sorteio.
    void desenharSorteio(std::ostream &o, int x, int y)
    {
        o << "<polygon points=\"" << x << ',' << y - 7 << ' '
          << x + 7 << ',' << y << ' '
          << x << ',' << y + 7 << ' '
          << x - 7 << ',' << y
          << "\" fill=\"#FF9F1C\" stroke=\"#222\" stroke-width=\"1.5\"/>\n";
    }

    // Pronta (esperando CPU): sem cor (requisito 2.1), so contorno tracejado.
    void desenharPronta(std::ostream &o, int x, int y, int w, int h)
    {
        o << "<rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << w << "\" height=\"" << h
          << "\" fill=\"none\" stroke=\"#8A8A8A\" stroke-dasharray=\"3,2\"/>\n";
    }

    // Triangulo de 10px. "base" e o y da base (apontando para cima) ou da ponta (para baixo).
    void desenharTriangulo(std::ostream &o, int x, int y, bool paraCima, const char *cor)
    {
        int ponta = paraCima ? y - 10 : y + 10;
        o << "<polygon points=\"" << x << ',' << ponta << ' ' << x - 5 << ',' << y << ' ' << x + 5 << ',' << y
          << "\" fill=\"" << cor << "\"/>\n";
    }

    // Prazo (deadline) perdido: "X" vermelho no instante em que estourou.
    void desenharPrazoPerdido(std::ostream &o, int x, int y)
    {
        o << "<path d=\"M" << x - 5 << ' ' << y - 5 << 'L' << x + 5 << ' ' << y + 5 << 'M' << x - 5 << ' ' << y + 5
          << 'L' << x + 5 << ' ' << y - 5 << "\" stroke=\"#D00000\" stroke-width=\"2.5\" fill=\"none\"/>\n";
    }

    // Chegada e termino ficam em zonas diferentes da faixa (baixo e cima) para nao se
    // sobreporem quando acontecem no mesmo instante.
    const char *COR_CHEGADA = "#1A7F37";
    const char *COR_TERMINO = "#111";
}

bool corParaRGB(const std::string &hex, int &r, int &g, int &b)
{
    std::string h = (!hex.empty() && hex[0] == '#') ? hex.substr(1) : hex;
    if (h.size() != 6 || !std::all_of(h.begin(), h.end(), [](unsigned char c)
                                      { return std::isxdigit(c); }))
        return false;
    return std::sscanf(h.c_str(), "%2x%2x%2x", &r, &g, &b) == 3;
}

bool exportarGanttSVG(const Simulador &simulador, const std::string &caminho, std::string &erro)
{
    const auto &gantt = simulador.getHistoricoGantt(); // gantt[tick][cpu] = id da tarefa (-1 = ociosa)
    const auto &eventos = simulador.getEventos();
    const int total = static_cast<int>(gantt.size());

    erro.clear();
    if (total == 0 || simulador.getTarefas().empty())
    {
        erro = "a simulacao ainda nao executou nenhum tick; nao ha nada para desenhar";
        return false;
    }

    // ---------- Linhas: ID maior no topo, ID menor colado no eixo X (requisito 2.5) ----------
    std::vector<const Tarefa *> ordem;
    for (const Tarefa &t : simulador.getTarefas())
        ordem.push_back(&t);
    std::sort(ordem.begin(), ordem.end(), [](const Tarefa *a, const Tarefa *b)
              { return a->getId() > b->getId(); });
    const int n = static_cast<int>(ordem.size());
    std::map<int, int> linha; // id da tarefa -> indice da linha
    for (int i = 0; i < n; i++)
        linha[ordem[i]->getId()] = i;

    // ---------- Estado de cada tarefa em cada tick ----------
    // est[linha][tick] = id da CPU que executou, PRONTA ou INATIVA.
    std::vector<std::vector<int>> est(n, std::vector<int>(total, INATIVA));
    for (int t = 0; t < total; t++)
        for (size_t cpu = 0; cpu < gantt[t].size(); cpu++)
            if (linha.count(gantt[t][cpu]))
                est[linha[gantt[t][cpu]]][t] = static_cast<int>(cpu);

    // Uma ativacao vai do evento CHEGADA ao evento TERMINO; nesse intervalo, os ticks sem CPU
    // sao espera na fila de prontas. (Os eventos estao em ordem cronologica.)
    auto marcarEspera = [&](int id, int ini, int fim)
    {
        for (int t = ini; t < std::min(fim, total); t++)
            if (est[linha[id]][t] == INATIVA)
                est[linha[id]][t] = PRONTA;
    };
    std::map<int, int> chegadaEmAberto; // id -> tick da chegada ainda sem termino
    for (const EventoGantt &ev : eventos)
    {
        if (!linha.count(ev.idTarefa))
            continue;
        if (ev.tipo == TipoEvento::CHEGADA)
            chegadaEmAberto[ev.idTarefa] = ev.tick;
        else if (ev.tipo == TipoEvento::TERMINO && chegadaEmAberto.count(ev.idTarefa))
        {
            marcarEspera(ev.idTarefa, chegadaEmAberto[ev.idTarefa], ev.tick);
            chegadaEmAberto.erase(ev.idTarefa);
        }
    }
    for (const auto &par : chegadaEmAberto) // ativacao em andamento quando a simulacao parou
        marcarEspera(par.first, par.second, total);

    // ---------- Dimensoes e legenda ----------
    // A legenda e desenhada antes porque o numero de linhas dela define a altura da imagem.
    const int largura = std::max(MARGEM_ESQ + total * LARGURA_TICK + 30, LARGURA_MINIMA);
    const int yEixo = MARGEM_TOPO + n * ALTURA_LINHA;
    const int yLegenda = yEixo + 52;

    std::ostringstream itens; // SVG dos itens da legenda
    int cx = 30, linhaLeg = 0;
    // Posiciona um item (amostra + texto) e quebra de linha quando nao cabe na largura.
    // O SVG nao mede texto, entao estima ~7px por caractere: melhor sobrar espaco que sobrepor.
    auto item = [&](const std::string &texto, const std::function<void(int, int)> &amostra)
    {
        int w = AMOSTRA + 34 + static_cast<int>(texto.size()) * 7;
        if (cx + w > largura - 20 && cx > 30)
        {
            cx = 30;
            linhaLeg++;
        }
        int yMeio = yLegenda + 44 + linhaLeg * LINHA_LEGENDA; // centro vertical da amostra
        amostra(cx, yMeio);
        itens << "<text x=\"" << cx + AMOSTRA + 8 << "\" y=\"" << yMeio + 4 << "\" font-size=\"12\">" << texto
              << "</text>\n";
        cx += w;
    };
    for (int i = n - 1; i >= 0; i--) // do menor ID para o maior
        item("Tarefa " + std::to_string(ordem[i]->getId()), [&, i](int x, int y)
             { desenharBarra(itens, x, y - 8, AMOSTRA, 16, *ordem[i], ""); });
    item("Pronta (aguardando CPU)", [&](int x, int y)
         { desenharPronta(itens, x, y - 8, AMOSTRA, 16); });
    item("Chegada (ativacao)", [&](int x, int y)
         { desenharTriangulo(itens, x + AMOSTRA / 2, y + 6, true, COR_CHEGADA); });
    item("Termino da execucao", [&](int x, int y)
         { desenharTriangulo(itens, x + AMOSTRA / 2, y - 6, false, COR_TERMINO); });
    item("Prazo perdido (deadline)", [&](int x, int y)
         { desenharPrazoPerdido(itens, x + AMOSTRA / 2, y); });
    item("Sorteio (desempate)", [&](int x, int y)
         { desenharSorteio(itens, x + AMOSTRA / 2, y); });

    const int alturaLegenda = 34 + (linhaLeg + 1) * LINHA_LEGENDA + 26;
    const int altura = yLegenda + alturaLegenda + 16;

    // ---------- Escrita do SVG ----------
    std::ofstream o(caminho, std::ios::binary); // binario: o Windows nao troca "\n" por "\r\n"
    if (!o)
    {
        erro = "nao foi possivel criar o arquivo \"" + caminho + "\" (verifique permissao de escrita na pasta)";
        return false;
    }
    o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << largura << "\" height=\"" << altura
      << "\" viewBox=\"0 0 " << largura << ' ' << altura << "\" font-family=\"Segoe UI, Arial, sans-serif\" fill=\"#222\">\n"
      << "<rect width=\"100%\" height=\"100%\" fill=\"#FFF\"/>\n";

    const size_t cpus = simulador.getCPUs().size();
    o << "<text x=\"20\" y=\"30\" font-size=\"18\" font-weight=\"bold\">Diagrama de Gantt - " << simulador.getAlgoritmo()
      << " - " << cpus << (cpus == 1 ? " CPU - " : " CPUs - ") << total << " ticks</text>\n";

    // Grade (uma linha vertical por tick, uma horizontal por tarefa) e rotulos Tn
    o << "<g stroke=\"#E3E3E3\">\n";
    for (int t = 0; t <= total; t++)
        o << "<line x1=\"" << MARGEM_ESQ + t * LARGURA_TICK << "\" y1=\"" << MARGEM_TOPO << "\" x2=\""
          << MARGEM_ESQ + t * LARGURA_TICK << "\" y2=\"" << yEixo + 5 << "\"/>\n";
    for (int r = 0; r <= n; r++)
        o << "<line x1=\"" << MARGEM_ESQ << "\" y1=\"" << MARGEM_TOPO + r * ALTURA_LINHA << "\" x2=\""
          << MARGEM_ESQ + total * LARGURA_TICK << "\" y2=\"" << MARGEM_TOPO + r * ALTURA_LINHA << "\"/>\n";
    o << "</g>\n";
    for (int r = 0; r < n; r++)
        o << "<text x=\"" << MARGEM_ESQ - 12 << "\" y=\"" << MARGEM_TOPO + r * ALTURA_LINHA + ALTURA_LINHA / 2 + 5
          << "\" text-anchor=\"end\" font-size=\"14\" font-weight=\"bold\">T" << ordem[r]->getId() << "</text>\n";

    // Barras: ticks consecutivos no mesmo estado viram um unico retangulo.
    // Trocar de CPU muda o estado, entao abre uma nova barra com o novo rotulo "Cn".
    for (int r = 0; r < n; r++)
    {
        const int y = MARGEM_TOPO + r * ALTURA_LINHA + FOLGA_BARRA;
        const int h = ALTURA_LINHA - 2 * FOLGA_BARRA;
        for (int t = 0; t < total;)
        {
            const int estado = est[r][t], ini = t;
            while (t < total && est[r][t] == estado)
                t++;
            const int x = MARGEM_ESQ + ini * LARGURA_TICK, w = (t - ini) * LARGURA_TICK;
            if (estado == PRONTA)
                desenharPronta(o, x, y, w, h);
            else if (estado >= 0)
                desenharBarra(o, x, y, w, h, *ordem[r], "C" + std::to_string(estado));
        }
    }

    // Eventos no instante exato em que acontecem (requisito 2.2), por cima das barras
    for (const EventoGantt &ev : eventos)
    {
        if (!linha.count(ev.idTarefa) || ev.tick < 0 || ev.tick > total)
            continue;
        const int x = MARGEM_ESQ + ev.tick * LARGURA_TICK, y = MARGEM_TOPO + linha[ev.idTarefa] * ALTURA_LINHA;
        if (ev.tipo == TipoEvento::CHEGADA)
            desenharTriangulo(o, x, y + ALTURA_LINHA - 3, true, COR_CHEGADA);
        else if (ev.tipo == TipoEvento::TERMINO)
            desenharTriangulo(o, x, y + 3, false, COR_TERMINO);
        else if (ev.tipo == TipoEvento::SORTEIO)
            desenharSorteio(o, x, y + ALTURA_LINHA / 2);
        else if (ev.tipo == TipoEvento::PRAZO_PERDIDO)
            desenharPrazoPerdido(o, x, y + ALTURA_LINHA / 2);
    }

    // Eixo do tempo (com 10000+ ticks, numeros de 5 digitos nao cabem: mostra 1 a cada 5)
    o << "<line x1=\"" << MARGEM_ESQ << "\" y1=\"" << yEixo << "\" x2=\"" << MARGEM_ESQ + total * LARGURA_TICK
      << "\" y2=\"" << yEixo << "\" stroke=\"#222\" stroke-width=\"1.5\"/>\n";
    for (int t = 0; t <= total; t += (total >= 10000 ? 5 : 1))
        o << "<text x=\"" << MARGEM_ESQ + t * LARGURA_TICK << "\" y=\"" << yEixo + 20
          << "\" text-anchor=\"middle\" font-size=\"11\">" << t << "</text>\n";
    o << "<text x=\"" << MARGEM_ESQ - 12 << "\" y=\"" << yEixo + 20
      << "\" text-anchor=\"end\" font-size=\"11\" fill=\"#555\">tempo (ticks)</text>\n";

    // Legenda: caixa, titulo, itens e nota sobre o rotulo "Cn"
    o << "<rect x=\"20\" y=\"" << yLegenda << "\" width=\"" << largura - 40 << "\" height=\"" << alturaLegenda
      << "\" fill=\"#FAFAFA\" stroke=\"#BBB\" rx=\"4\"/>\n"
      << "<text x=\"30\" y=\"" << yLegenda + 22 << "\" font-size=\"13\" font-weight=\"bold\">Legenda</text>\n"
      << itens.str() << "<text x=\"30\" y=\"" << yLegenda + alturaLegenda - 10
      << "\" font-size=\"11\" fill=\"#555\">O rotulo \"Cn\" dentro de uma barra indica a CPU em que a tarefa executou.</text>\n"
      << "</svg>\n";

    o.flush();
    if (!o)
    {
        erro = "falha ao gravar o arquivo \"" + caminho + "\" (disco cheio ou sem permissao?)";
        return false;
    }
    return true;
}
