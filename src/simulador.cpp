#include "header/simulador.hpp"
#include <algorithm>

using namespace std;

Simulador::Simulador(int qtde_cpus, int quantum, string algoritmo_escalonamento)
    : qtde_cpus(qtde_cpus), quantum(quantum),
      algoritmo_escalonamento(algoritmo_escalonamento) {}

Simulador::~Simulador() {}

void Simulador::criaEscalonador()
{
    // Instancia o escalonador pelo nome, consultando o registro de algoritmos (requisito 4.2):
    // o simulador nao conhece RM/EDF diretamente, entao novos algoritmos nao exigem mexer aqui.
    escalonador = criarEscalonador(algoritmo_escalonamento);
    if (!escalonador)
    {
        // criarEscalonador devolve nullptr se o nome nao esta registrado; o escalonador
        // anterior ja foi descartado pela atribuicao acima, entao nao e reaproveitado.
        cerr << "Erro: algoritmo de escalonamento desconhecido: "
             << algoritmo_escalonamento << endl;
    }
}

void Simulador::criaCPUS()
{
    cpus.clear();
    // Cria a quantidade de CPUs definida na configuração, atribuindo um ID a cada uma.
    for (int i = 0; i < qtde_cpus; i++)
        cpus.emplace_back(i);
}

void Simulador::criaTarefas()
{
    // Cria as tarefas iniciais com seus respectivos parâmetros.
    // tarefas.emplace_back(1, "FF0000", 0, 3, 5, 5);
    // tarefas.emplace_back(2, "00FF00", 0, 4, 10, 10);
    // tarefas.emplace_back(3, "0000FF", 2, 2, 7, 7);

    // As tarefas vêm sempre do arquivo de configuração (guardadas em tarefasIniciais)
    // atribui = e não acrescenta emplace_back. com o emplace_back, cada reset duplicaria as tarefas

    tarefas = tarefasIniciais; // copia, restaurando o estado inicial de cada tarefa
}

void Simulador::atualizaFilaDeProntas()
{
    filaDeProntas.clear();

    // Mantém na fila apenas os IDs das tarefas que estão prontas para execução.
    for (const Tarefa &tarefa : tarefas)
        if (tarefa.getEstado() == EstadoTarefa::PRONTA)
            filaDeProntas.push_back(tarefa.getId());
}

void Simulador::avancaTick()
{
    int tickAtual = relogio.getTick();

    // Prazo perdido: so adiciona a marcacao. a tarefa nao é interrompida e continua executando
    for (Tarefa &tarefa : tarefas)
    {
        bool jaPerdeu = tarefa.getPerdeuPrazo();
        tarefa.verificarPrazo(tickAtual);
        if (!jaPerdeu && tarefa.getPerdeuPrazo())
            eventos.push_back({tickAtual, tarefa.getId(), TipoEvento::PRAZO_PERDIDO});
    }

    // Ativa as tarefas que chegam neste instante.
    for (Tarefa &tarefa : tarefas)
    {
        if (tarefa.deveExecutar(tickAtual))
        {
            tarefa.ativar(tickAtual);
            eventos.push_back({tickAtual, tarefa.getId(), TipoEvento::CHEGADA});
        }
    }

    // Guarda as execucoes concluidas antes do tick, para detectar quem terminou
    vector<int> execAntes;
    for (const Tarefa &tarefa : tarefas)
        execAntes.push_back(tarefa.getExecucoes());
    // ex: para nao contar uma tarefa que ja tinha terminado

    // Executa as tarefas durante um tick
    executarSimulacao();

    // Registra términos ocorridos ao final do tick.
    for (size_t i = 0; i < tarefas.size(); i++)
    {
        if (tarefas[i].getExecucoes() > execAntes[i])
        {
            eventos.push_back({tickAtual + 1,
                               tarefas[i].getId(),
                               TipoEvento::TERMINO});
        }
    }

    // Avança o relógio.
    relogio.incrementa();

    // Salva o estado final do tick.
    salvaHistoricoTick();
}

void Simulador::retrocedeTick()
{
    // Não permite voltar antes do estado inicial.
    if (historico.size() <= 1)
        return;

    // Remove o estado atual.
    historico.pop_back();

    // Cada tick executado deve corresponder a uma linha do Gantt.
    if (!historicoGantt.empty())
        historicoGantt.pop_back();

    const EstadoSistema &estado = historico.back();

    // Restaura tarefas e relógio.
    tarefas = estado.tarefas;
    relogio.setTick(estado.tick);

    // Restaura os demais dados do sistema.
    filaDeProntas = estado.filaDeProntas;
    eventos = estado.eventos;
    quantum = estado.quantum;
    qtde_cpus = estado.quantidadeCPUs > 0 ? estado.quantidadeCPUs : static_cast<int>(estado.cpus.size());
    algoritmo_escalonamento = estado.algoritmo.empty() ? algoritmo_escalonamento : estado.algoritmo;
    escalonador.reset();
    criaEscalonador();

    // Reconstrói as CPUs usando os IDs das tarefas salvos.
    cpus.clear();
    cpus.reserve(estado.cpus.size());

    for (const EstadoCPU &salva : estado.cpus)
    {
        cpus.emplace_back(salva.id);
        CPU &cpu = cpus.back();

        if (!salva.ligado)
            cpu.desligar();

        // Recupera o tempo acumulado com a CPU desligada.
        cpu.incrementaTempoDesligado(salva.tempoDesligado);

        // Localiza e reassocia à CPU a tarefa registrada no histórico.
        if (salva.idTarefaAtual != -1)
        {
            for (Tarefa &tarefa : tarefas)
            {
                if (tarefa.getId() == salva.idTarefaAtual)
                {
                    // A atribuição exige que a tarefa esteja pronta.
                    tarefa.setEstado(EstadoTarefa::PRONTA);
                    cpu.atribuirTarefa(&tarefa);
                    break;
                }
            }
        }
    }

    // Atribuir uma tarefa à CPU pode modificar seus dados.
    // Restaura os estados e IDs de CPU salvos no histórico.
    for (const Tarefa &salva : estado.tarefas)
    {
        for (Tarefa &atual : tarefas)
        {
            if (atual.getId() == salva.getId())
            {
                atual.setEstado(salva.getEstado());
                atual.setIdCPU(salva.getIdCPU());
                break;
            }
        }
    }
}

void Simulador::escalonarCPUs()
{
    // Candidatas: toda tarefa pronta ou executando, em qualquer CPU.
    // As que estao executando continuam EXECUTANDO durante a selecao,
    // entao o desempate favorece quem ja estava rodando.
    vector<Tarefa *> restantes;
    for (Tarefa &tarefa : tarefas)
        if (tarefa.getEstado() == EstadoTarefa::PRONTA ||
            tarefa.getEstado() == EstadoTarefa::EXECUTANDO)
            restantes.push_back(&tarefa);

    // Escolhe as N tarefas de maior prioridade (N = quantidade de CPUs)
    // Se for registrar o evento de sorteio, leia escalonador->houveSorteio() logo apos cada selecionar().
    vector<Tarefa *> escolhidas;
    for (size_t k = 0; k < cpus.size() && !restantes.empty(); k++)
    {
        // Semente do sorteio de desempate derivada do tick e da posicao k (nao de rand()):
        // o mesmo estado do sistema sempre sorteia igual, entao avancar depois de retroceder
        // reproduz exatamente a simulacao anterior.
        escalonador->definirSemente(static_cast<unsigned long long>(relogio.getTick()) * 1000003ULL + k);
        Tarefa *melhor = escalonador->selecionar(restantes);
        if (!melhor)
            break;

        // Registra o sorteio se a tarefa nao estava executando.
        // Verifica houveSorteio() logo apos selecionar(), pois a flag e reiniciada.
        if (escalonador->houveSorteio() && melhor->getEstado() != EstadoTarefa::EXECUTANDO)
        {
            eventos.push_back({relogio.getTick(), melhor->getId(), TipoEvento::SORTEIO});
        }

        escolhidas.push_back(melhor);
        restantes.erase(find(restantes.begin(), restantes.end(), melhor));
    }

    // Preempcao: libera as CPUs cuja tarefa nao foi escolhida.
    for (CPU &cpu : cpus)
    {
        Tarefa *atual = cpu.getTarefaAtual();
        if (atual && find(escolhidas.begin(), escolhidas.end(), atual) == escolhidas.end())
            cpu.liberar();
    }

    // As escolhidas que ainda nao estao em nenhuma CPU ocupam uma CPU livre.
    // As que ja estao executando ficam onde estao (sem migracao entre CPUs)
    for (Tarefa *tarefa : escolhidas)
    {
        if (tarefa->getEstado() == EstadoTarefa::EXECUTANDO)
            continue;

        for (CPU &cpu : cpus)
        {
            if (cpu.estaLivre())
            {
                cpu.atribuirTarefa(tarefa);
                break;
            }
        }
    }

    // CPU sem tarefa neste tick e desligada (requisito 1.2)
    for (CPU &cpu : cpus)
        if (!cpu.getTarefaAtual())
            cpu.desligar();
}

// Verifica se a cor esta no formato hexadecimal.
static bool normalizarCor(const string &cor, string &novaCor)
{
    novaCor = cor;

    // Remove o # caso exista.
    if (!novaCor.empty() && novaCor[0] == '#')
        novaCor.erase(0, 1);

    // A cor deve ter 6 caracteres.
    if (novaCor.size() != 6)
        return false;

    // Verifica se todos os caracteres sao hexadecimais.
    for (char c : novaCor)
    {
        if (!((c >= '0' && c <= '9') ||
              (c >= 'A' && c <= 'F') ||
              (c >= 'a' && c <= 'f')))
            return false;
    }

    return true;
}

/*
========================================================================
======================EXECUCAO DA SIMULACAO========================================
========================================================================
*/

void Simulador::executarSimulacao()
{
    // Garante que exista um escalonador válido antes de executar.
    if (!escalonador)
        criaEscalonador();

    if (!escalonador)
    {
        cerr << "Erro: algoritmo de escalonamento inválido." << endl;
        return;
    }

    // Decide qual tarefa ocupa cada CPU neste tick (escalonamento global entre todas as CPUs).
    // Ela também desliga as CPUs que ficaram sem tarefa (requisito 1.2).
    escalonarCPUs();

    // Registra o estado das CPUs antes de executar o tick.
    salvarTickGantt();

    // Soma 1 tick ao tempo desligado de cada CPU que ficou sem tarefa neste tick.
    for (CPU &cpu : cpus)
        if (!cpu.getLigado())
            cpu.incrementaTempoDesligado(1);

    // Avança a execução das tarefas atribuídas às CPUs.
    for (CPU &cpu : cpus)
    {
        if (cpu.getLigado())
            cpu.executarTick();
    }

    // O histórico deve refletir o estado posterior à execução.
    atualizaFilaDeProntas();
}

/*====================================================================
======================================================================
*/

void Simulador::salvaHistoricoTick()
{
    // Cria uma cópia do estado do sistema para o tick atual.
    EstadoSistema estado;
    estado.tick = relogio.getTick();
    estado.tarefas = tarefas;
    estado.quantum = quantum;
    estado.quantidadeCPUs = qtde_cpus;
    estado.algoritmo = algoritmo_escalonamento;
    estado.filaDeProntas = filaDeProntas;
    estado.eventos = eventos; // eventos ocorridos ate este tick

    // Salva os dados de cada CPU sem armazenar ponteiros para tarefas.
    // O ID da tarefa permite reconstruir a atribuição durante o retrocesso.
    for (const CPU &cpu : cpus)
    {
        EstadoCPU salva;
        salva.id = cpu.getId();
        salva.idTarefaAtual = cpu.getTarefaAtual()
                                  ? cpu.getTarefaAtual()->getId()
                                  : -1;
        salva.ligado = cpu.getLigado();
        salva.emUso = cpu.getEmUso();
        salva.tempoDesligado = cpu.getTempoDesligado();

        estado.cpus.push_back(salva);
    }

    // Armazena o estado completo no histórico.
    historico.push_back(estado);
}

int Simulador::getTickAtual() const
{
    return relogio.getTick();
}

const vector<vector<int>> &Simulador::getHistoricoGantt() const
{
    return historicoGantt;
}

const vector<CPU> &Simulador::getCPUs() const
{
    return cpus;
}

const vector<Tarefa> &Simulador::getTarefas() const
{
    return tarefas;
}

void Simulador::salvarTickGantt()
{
    vector<int> linha;

    // Cada posição representa uma CPU: salva o ID da tarefa executada
    // ou -1 quando a CPU está ociosa.
    for (const CPU &cpu : cpus)
        linha.push_back(cpu.getTarefaAtual()
                            ? cpu.getTarefaAtual()->getId()
                            : -1);

    // Cada linha corresponde a um tick da simulação.
    historicoGantt.push_back(linha);
}

void Simulador::reiniciar()
{
    // Limpa os dados da execução anterior.
    historico.clear();
    historicoGantt.clear();
    filaDeProntas.clear();

    // Restaura as tarefas ao estado inicial.
    criaTarefas();

    // Reinicia os componentes do sistema.
    cpus.clear();
    eventos.clear();
    escalonador.reset();
    relogio = Relogio();

    // Recria as CPUs e o escalonador com os parâmetros atuais.
    criaCPUS();
    criaEscalonador();

    // Ativa as tarefas que chegam no tick inicial.
    for (Tarefa &tarefa : tarefas)
    {
        if (tarefa.deveExecutar(0))
        {
            tarefa.ativar(0);
            eventos.push_back({0, tarefa.getId(), TipoEvento::CHEGADA}); // registra a chegada no instante inicial
        }
    }

    // Atualiza a fila e registra o estado inicial.
    atualizaFilaDeProntas();
    salvaHistoricoTick();
}

// executar tudo sozinho

bool Simulador::todasTerminadas() const
{
    // A simulacao acaba quando toda tarefa periodica ja cumpriu suas 10 execucoes.
    for (const Tarefa &tarefa : tarefas)
        if (tarefa.getEstado() != EstadoTarefa::TERMINADA)
            return false;

    return true;
}

bool Simulador::executarCompleto(int limiteTicks)
{
    // O limite conta os ticks desta chamada (nao o tick absoluto), assim funciona
    // tambem quando o usuario ja avancou alguns ticks no passo-a-passo.
    int ticksExecutados = 0;

    while (!todasTerminadas())
    {
        // limite para evitar loop infinito em caso de bug
        if (ticksExecutados >= limiteTicks)
            return false;

        avancaTick(); // avanca tick até terminar td
        ticksExecutados++;
    }
    return true;
}

void Simulador::definirTarefas(const std::vector<Tarefa> &tarefas)
{
    tarefasIniciais = tarefas;

    // Ordena por ID crescente: o Gantt desenha o menor id mais perto do eixo x,
    // e o arquivo de configuracao pode trazer as tarefas em qualquer ordem.
    sort(tarefasIniciais.begin(), tarefasIniciais.end(),
         [](const Tarefa &a, const Tarefa &b)
         { return a.getId() < b.getId(); });

    this->tarefas = tarefasIniciais;
}

void Simulador::atualizarHistoricoAtual()
{
    // A configuração é uma alteração no instante atual, não um novo tick.
    // Atualiza apenas o snapshot do tick corrente; os snapshots anteriores ficam intactos.
    if (historico.empty())
        return;

    EstadoSistema &estado = historico.back();
    estado.tick = relogio.getTick();
    estado.tarefas = tarefas;
    estado.cpus.clear();
    for (const CPU &cpu : cpus)
    {
        EstadoCPU salva;
        salva.id = cpu.getId();
        salva.idTarefaAtual = cpu.getTarefaAtual() ? cpu.getTarefaAtual()->getId() : -1;
        salva.ligado = cpu.getLigado();
        salva.emUso = cpu.getEmUso();
        salva.tempoDesligado = cpu.getTempoDesligado();
        estado.cpus.push_back(salva);
    }
    estado.quantum = quantum;
    estado.quantidadeCPUs = qtde_cpus;
    estado.algoritmo = algoritmo_escalonamento;
    estado.filaDeProntas = filaDeProntas;
    estado.eventos = eventos;
}

void Simulador::definirAlgoritmo(const std::string &algoritmo)
{
    // So aceita algoritmos registrados (RM, EDF e qualquer outro incluido no registro).
    if (!algoritmoRegistrado(algoritmo))
        return;
    if (algoritmo_escalonamento == algoritmo)
        return;

    algoritmo_escalonamento = algoritmo;
    // Troca o escalonador sem reiniciar o relógio, as tarefas ou o Gantt.
    escalonador.reset();
    criaEscalonador();
    atualizarHistoricoAtual();
}

void Simulador::definirQuantidadeCPUs(int quantidade)
{
    if (quantidade < 1 || quantidade == qtde_cpus)
        return;

    // Se CPUs forem removidas, suas tarefas voltam à fila de prontas sem consumir tempo.
    if (quantidade < static_cast<int>(cpus.size()))
    {
        for (size_t i = static_cast<size_t>(quantidade); i < cpus.size(); ++i)
            cpus[i].liberar();
        cpus.erase(cpus.begin() + quantidade, cpus.end());
    }
    else
    {
        for (int i = static_cast<int>(cpus.size()); i < quantidade; ++i)
            cpus.emplace_back(i);
    }

    qtde_cpus = quantidade;
    atualizaFilaDeProntas();
    atualizarHistoricoAtual();
}

void Simulador::definirQuantum(int novoQuantum)
{
    if (novoQuantum >= 1 && novoQuantum <= 1000 && quantum != novoQuantum)
    {
        quantum = novoQuantum;
        atualizarHistoricoAtual();
    }
}

// um retrocesso/avanço restaure também a alteração manual feita pelo usuário.
// Edita a tarefa e salva as alteracoes no historico.
bool Simulador::editarTarefa(int id, const EdicaoTarefa &edicao, string &erro)
{
    auto it = find_if(tarefas.begin(), tarefas.end(), [id](const Tarefa &t)
                      { return t.getId() == id; });
    if (it == tarefas.end())
    {
        erro = "A tarefa selecionada não existe.";
        return false;
    }

    EstadoTarefa novoEstado = edicao.estado;
    int novoTempoRestante = edicao.tempoRestante;

    // Verifica os valores informados.
    string novaCor;
    if (!normalizarCor(edicao.cor, novaCor))
    {
        erro = "A cor deve ter 6 digitos hexadecimais (RRGGBB).";
        return false;
    }
    if (edicao.ingresso < 0)
    {
        erro = "O ingresso nao pode ser negativo.";
        return false;
    }
    if (edicao.duracao <= 0)
    {
        erro = "A duracao deve ser maior que zero.";
        return false;
    }
    if (edicao.periodo <= 0)
    {
        erro = "O periodo deve ser maior que zero.";
        return false;
    }
    if (edicao.prazo <= 0)
    {
        erro = "O prazo deve ser maior que zero.";
        return false;
    }

    // O ingresso so pode mudar antes da primeira ativacao.
    if (edicao.ingresso != it->getIngresso())
    {
        if (it->getAtivacoes() > 0)
        {
            erro = "Nao e possivel alterar o ingresso depois da primeira ativacao.";
            return false;
        }
        if (edicao.ingresso < relogio.getTick())
        {
            erro = "O novo ingresso nao pode ser anterior ao tick atual.";
            return false;
        }
    }

    // Se o usuário mudou a duração mas não mexeu no tempo restante, acompanha a nova duração
    if (edicao.tempoRestante == it->getTempoRestante() && edicao.duracao > 0 && edicao.duracao != it->getDuracao())
    {
        if (it->getEstado() == EstadoTarefa::NOVA)
            novoTempoRestante = edicao.duracao;
        else
            novoTempoRestante = min(novoTempoRestante, edicao.duracao);
    }

    // -----------------

    if (novoTempoRestante < 0 || novoTempoRestante > edicao.duracao)
    {
        erro = "O tempo restante deve estar entre 0 e a duracao da tarefa.";
        return false;
    }
    if (novoEstado == EstadoTarefa::AGUARDANDO && novoTempoRestante != 0)
    {
        erro = "Uma tarefa aguardando precisa ter tempo restante igual a zero.";
        return false;
    }
    if (novoEstado == EstadoTarefa::TERMINADA)
        novoTempoRestante = 0;
    if ((novoEstado == EstadoTarefa::PRONTA || novoEstado == EstadoTarefa::EXECUTANDO ||
         novoEstado == EstadoTarefa::SUSPENSA) &&
        novoTempoRestante == 0)
    {
        erro = "Uma tarefa pronta, executando ou suspensa precisa ter tempo restante maior que zero.";
        return false;
    }

    // Verifica se existe CPU livre antes de colocar a tarefa em execucao.
    if (novoEstado == EstadoTarefa::EXECUTANDO && it->getIdCPU() < 0)
    {
        auto cpuLivre = find_if(cpus.begin(), cpus.end(), [](const CPU &cpu)
                                { return cpu.estaLivre() != 0; });
        if (cpuLivre == cpus.end())
        {
            erro = "Nao ha CPU livre para colocar esta tarefa em execucao.";
            return false;
        }
    }

    // Libera a CPU caso a tarefa deixe de executar.
    if (it->getIdCPU() >= 0 && novoEstado != EstadoTarefa::EXECUTANDO)
    {
        for (CPU &cpu : cpus)
            if (cpu.getId() == it->getIdCPU() && cpu.getTarefaAtual() == &*it)
            {
                cpu.liberar();
                break;
            }
    }

    // Aplica o novo estado.
    if (novoEstado == EstadoTarefa::EXECUTANDO && it->getIdCPU() < 0)
    {
        auto cpuLivre = find_if(cpus.begin(), cpus.end(), [](const CPU &cpu)
                                { return cpu.estaLivre() != 0; });
        it->setEstado(EstadoTarefa::PRONTA);
        it->setTempoRestante(novoTempoRestante);
        cpuLivre->atribuirTarefa(&*it);
    }
    else
    {
        it->setEstado(novoEstado);
        it->setIdCPU(novoEstado == EstadoTarefa::EXECUTANDO ? it->getIdCPU() : -1);
        it->setTempoRestante(novoTempoRestante);
    }

    if (novoEstado == EstadoTarefa::TERMINADA)
        it->setIdCPU(-1);

    // Atualiza o prazo e a proxima ativacao.
    if (edicao.prazo != it->getPrazo() && it->getPrazoAbsoluto() >= 0)
        it->setPrazoAbsoluto(it->getPrazoAbsoluto() - it->getPrazo() + edicao.prazo);

    if (edicao.periodo != it->getPeriodo() && it->getAtivacoes() > 0)
        it->setProximaAtivacao(it->getProximaAtivacao() + edicao.periodo - it->getPeriodo());

    if (edicao.ingresso != it->getIngresso())
    {
        it->setIngresso(edicao.ingresso);
        it->setProximaAtivacao(edicao.ingresso);
    }

    // Atualiza as caracteristicas da tarefa.
    it->setCor(novaCor);
    it->setDuracao(edicao.duracao);
    it->setPeriodo(edicao.periodo);
    it->setPrazo(edicao.prazo);

    // Atualiza a copia inicial para manter as mudancas ao resetar.
    for (Tarefa &inicial : tarefasIniciais)
    {
        if (inicial.getId() != id)
            continue;

        if (edicao.ingresso != inicial.getIngresso())
        {
            inicial.setIngresso(edicao.ingresso);
            inicial.setProximaAtivacao(edicao.ingresso);
        }

        inicial.setCor(novaCor);
        inicial.setDuracao(edicao.duracao);
        inicial.setTempoRestante(edicao.duracao);
        inicial.setPeriodo(edicao.periodo);
        inicial.setPrazo(edicao.prazo);
        break;
    }

    atualizaFilaDeProntas();

    if (!historico.empty())
    {
        // Salva o estado atual sem alterar o Gantt ja executado.
        historico.pop_back();
        salvaHistoricoTick();
    }

    erro.clear();
    return true;
}


/*
========================================================================
============ EDICAO DO CONJUNTO DE TAREFAS EM TEMPO DE EXECUCAO ========
========================================================================
*/

vector<int> Simulador::idsTarefasNasCPUs() const
{
    vector<int> ids;
    for (const CPU &cpu : cpus)
        ids.push_back(cpu.getTarefaAtual() ? cpu.getTarefaAtual()->getId() : -1);
    return ids;
}

void Simulador::religarCPUs(const vector<int> &idsPorCpu)
{
    // Depois de mexer no vector 'tarefas' os objetos mudaram de endereco: procura cada tarefa
    // pelo id e reaponta a CPU que a executava.
    for (size_t i = 0; i < cpus.size() && i < idsPorCpu.size(); i++)
    {
        if (idsPorCpu[i] < 0)
            continue;
        for (Tarefa &tarefa : tarefas)
            if (tarefa.getId() == idsPorCpu[i])
            {
                cpus[i].reapontar(&tarefa);
                break;
            }
    }
}

void Simulador::regravarHistoricoAtual()
{
    // Mesma tecnica do editarTarefa: troca o ultimo snapshot pelo estado atual, sem alterar o
    // Gantt ja executado, para que retroceder/avancar preserve a alteracao manual.
    if (historico.empty())
        return;
    historico.pop_back();
    salvaHistoricoTick();
}

bool Simulador::adicionarTarefa(int id, const string &cor, int ingresso, int duracao,
                                int periodo, int prazo, string &erro)
{
    // Validacoes: cada uma devolve uma justificativa clara (requisito 3.1/3.4).
    if (id <= 0)
    {
        erro = "O ID deve ser maior que zero.";
        return false;
    }
    for (const Tarefa &t : tarefas)
        if (t.getId() == id)
        {
            erro = "Ja existe uma tarefa com o ID " + to_string(id) + ".";
            return false;
        }
    string novaCor;
    if (!normalizarCor(cor, novaCor))
    {
        erro = "A cor deve ter 6 digitos hexadecimais (RRGGBB).";
        return false;
    }
    if (ingresso < relogio.getTick())
    {
        // Uma tarefa nao pode "chegar no passado": o Gantt desses ticks ja foi desenhado.
        erro = "O ingresso nao pode ser anterior ao tick atual (" + to_string(relogio.getTick()) + ").";
        return false;
    }
    if (duracao <= 0 || periodo <= 0 || prazo <= 0)
    {
        erro = "Duracao, periodo e prazo devem ser maiores que zero (somente tarefas periodicas no Projeto A).";
        return false;
    }

    const vector<int> idsCpu = idsTarefasNasCPUs();

    // Insere na simulacao atual e na copia inicial (para a tarefa sobreviver a "Resetar").
    tarefas.emplace_back(id, novaCor, ingresso, duracao, periodo, prazo);
    tarefasIniciais.emplace_back(id, novaCor, ingresso, duracao, periodo, prazo);

    // Mantem a ordem por ID: o Gantt e a legenda dependem dela.
    auto porId = [](const Tarefa &a, const Tarefa &b)
    { return a.getId() < b.getId(); };
    sort(tarefas.begin(), tarefas.end(), porId);
    sort(tarefasIniciais.begin(), tarefasIniciais.end(), porId);

    religarCPUs(idsCpu); // o vector foi realocado/reordenado
    atualizaFilaDeProntas();
    regravarHistoricoAtual();

    erro.clear();
    return true;
}

bool Simulador::removerTarefa(int id, string &erro)
{
    auto it = find_if(tarefas.begin(), tarefas.end(), [id](const Tarefa &t)
                      { return t.getId() == id; });
    if (it == tarefas.end())
    {
        erro = "A tarefa selecionada nao existe.";
        return false;
    }
    if (tarefas.size() == 1)
    {
        erro = "O sistema precisa ter pelo menos uma tarefa.";
        return false;
    }
    if (it->getAtivacoes() > 0)
    {
        // Ela ja aparece no Gantt/historico; remover deixaria o grafico inconsistente.
        erro = "A tarefa ja foi ativada e consta no historico da simulacao. "
               "Retroceda ate antes do ingresso dela ou reinicie para remove-la.";
        return false;
    }

    const vector<int> idsCpu = idsTarefasNasCPUs();

    tarefas.erase(it);
    tarefasIniciais.erase(remove_if(tarefasIniciais.begin(), tarefasIniciais.end(),
                                    [id](const Tarefa &t)
                                    { return t.getId() == id; }),
                          tarefasIniciais.end());

    religarCPUs(idsCpu);
    atualizaFilaDeProntas();
    regravarHistoricoAtual();

    erro.clear();
    return true;
}

void Simulador::reconfigurar(int novasCpus, int novoQuantum, const string &algoritmo,
                             const vector<Tarefa> &novasTarefas)
{
    // Aplica os parametros novos e recomeca do zero (historico, relogio, CPUs e tarefas).
    qtde_cpus = novasCpus < 1 ? 1 : novasCpus;
    quantum = novoQuantum < 1 ? 1 : novoQuantum;
    if (algoritmoRegistrado(algoritmo))
        algoritmo_escalonamento = algoritmo;
    definirTarefas(novasTarefas);
    reiniciar();
}
