#include "header/simulador.hpp"
#include <algorithm>

using namespace std;

Simulador::Simulador(int qtde_cpus, int quantum, string algoritmo_escalonamento)
    : qtde_cpus(qtde_cpus), quantum(quantum),
      algoritmo_escalonamento(algoritmo_escalonamento) {}

Simulador::~Simulador() {}

void Simulador::criaEscalonador()
{
    // Instancia o escalonador de acordo com o algoritmo escolhido.
    if (algoritmo_escalonamento == "RM")
        escalonador = make_unique<EscalonadorRM>();
    else if (algoritmo_escalonamento == "EDF")
        escalonador = make_unique<EscalonadorEDF>();
    else
    {
        // Impede o uso de um escalonador anterior.
        escalonador.reset();
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

void Simulador::definirAlgoritmo(const std::string &algoritmo)
{
    algoritmo_escalonamento = algoritmo;
}

void Simulador::definirQuantidadeCPUs(int quantidade)
{
    if (quantidade >= 1 && quantidade <= 8)
        qtde_cpus = quantidade;
}

void Simulador::definirQuantum(int novoQuantum)
{
    if (novoQuantum >= 1 && novoQuantum <= 10)
        quantum = novoQuantum;
}
