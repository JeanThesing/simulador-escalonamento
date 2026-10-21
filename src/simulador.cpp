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
    tarefas.emplace_back(1, "FF0000", 0, 3, 5, 5);
    tarefas.emplace_back(2, "00FF00", 0, 4, 10, 10);
    tarefas.emplace_back(3, "0000FF", 2, 2, 7, 7);
}

void Simulador::atualizaFilaDeProntas()
{
    filaDeProntas.clear();

    // Mantém na fila apenas os IDs das tarefas que estão prontas para execução.
    for (const Tarefa& tarefa : tarefas)
        if (tarefa.getEstado() == EstadoTarefa::PRONTA)
            filaDeProntas.push_back(tarefa.getId());
}

void Simulador::avancaTick()
{
    relogio.incrementa();
    int tickAtual = relogio.getTick();

    // Verifica se alguma tarefa deve ser ativada neste instante da simulação.
    for (Tarefa& tarefa : tarefas)
        if (tarefa.deveExecutar(tickAtual))
            tarefa.ativar(tickAtual);

    executarSimulacao();
}

void Simulador::retrocedeTick()
{
    // O primeiro registro representa o estado inicial e não pode ser removido.
    if (historico.size() <= 1)
        return;

    // Remove o estado atual e a última linha do gráfico de Gantt.
    historico.pop_back();
    if (!historicoGantt.empty())
        historicoGantt.pop_back();

    // Recupera o estado anterior e restaura o relógio e as tarefas.
    const EstadoSistema& estado = historico.back();
    tarefas = estado.tarefas;
    cpus.clear();
    relogio.setTick(estado.tick);

    // Reconstrói as CPUs a partir dos dados salvos, pois os ponteiros
    // das tarefas precisam apontar para os objetos atuais do vetor tarefas.
    for (const EstadoCPU& salva : estado.cpus)
    {
        cpus.emplace_back(salva.id);
        CPU& cpu = cpus.back();

        if (!salva.ligado)
            cpu.desligar();

        // Recupera o tempo acumulado com a CPU desligada.
        cpu.incrementaTempoDesligado(salva.tempoDesligado);

        // Localiza e reassocia à CPU a tarefa registrada no histórico.
        if (salva.idTarefaAtual != -1)
        {
            for (Tarefa& tarefa : tarefas)
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

    // A reconstrução das CPUs pode alterar estados das tarefas.
    // Por isso, restaura os estados e IDs de CPU exatamente como foram salvos.
    for (const Tarefa& salva : estado.tarefas)
    {
        for (Tarefa& atual : tarefas)
        {
            if (atual.getId() == salva.getId())
            {
                atual.setEstado(salva.getEstado());
                atual.setIdCPU(salva.getIdCPU());
                break;
            }
        }
    }

    // Recupera os demais dados da configuração naquele instante.
    filaDeProntas = estado.filaDeProntas;
    quantum = estado.quantum;
}

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

    // Impede que a mesma tarefa seja atribuída a mais de uma CPU no tick.
    vector<Tarefa*> tarefasAtribuidas;

    for (CPU& cpu : cpus)
    {
        if (!cpu.getLigado())
            cpu.ligar();

        vector<Tarefa*> candidatas;

        // Monta a lista de tarefas que esta CPU pode executar.
        for (Tarefa& tarefa : tarefas)
        {
            bool nestaCPU = cpu.getTarefaAtual() == &tarefa;
            bool emOutraCPU =
                tarefa.getEstado() == EstadoTarefa::EXECUTANDO && !nestaCPU;

            // Verifica se outra CPU já recebeu esta tarefa durante o escalonamento.
            bool jaAtribuida =
                find(tarefasAtribuidas.begin(), tarefasAtribuidas.end(), &tarefa)
                != tarefasAtribuidas.end();

            // Tarefas em execução em outra CPU ou já atribuídas são descartadas.
            if (emOutraCPU || jaAtribuida)
                continue;

            // Inclui tarefas prontas e a tarefa que já ocupa esta CPU,
            // permitindo que o escalonador decida se ela deve continuar executando.
            if (tarefa.getEstado() == EstadoTarefa::PRONTA || nestaCPU)
                candidatas.push_back(&tarefa);
        }

        // O algoritmo RM ou EDF escolhe a tarefa de maior prioridade.
        // A decisão também pode manter a tarefa atual ou deixar a CPU ociosa.
        Tarefa* escolhida = escalonador->selecionar(candidatas);

        // Se a escolha mudar, libera a tarefa anterior antes de atribuir a nova.
        if (escolhida != cpu.getTarefaAtual())
        {
            cpu.liberar();

            if (escolhida)
                cpu.atribuirTarefa(escolhida);
        }

        // Registra a tarefa escolhida para evitar atribuições duplicadas.
        if (cpu.getTarefaAtual())
            tarefasAtribuidas.push_back(cpu.getTarefaAtual());
    }

    // Atualiza a fila de prontas após as decisões do escalonador.
    atualizaFilaDeProntas();

    // Registra quais tarefas serão executadas em cada CPU neste tick.
    salvarTickGantt();

    // Avança a execução das tarefas atribuídas às CPUs.
    for (CPU& cpu : cpus)
        cpu.executarTick();

    // Salva o estado resultante para permitir o retrocesso da simulação.
    salvaHistoricoTick();
}

void Simulador::salvaHistoricoTick()
{
    // Cria uma cópia do estado do sistema para o tick atual.
    EstadoSistema estado;
    estado.tick = relogio.getTick();
    estado.tarefas = tarefas;
    estado.quantum = quantum;
    estado.filaDeProntas = filaDeProntas;

    // Salva os dados de cada CPU sem armazenar ponteiros para tarefas.
    // O ID da tarefa permite reconstruir a atribuição durante o retrocesso.
    for (const CPU& cpu : cpus)
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

const vector<vector<int>>& Simulador::getHistoricoGantt() const
{
    return historicoGantt;
}

const vector<CPU>& Simulador::getCPUs() const
{
    return cpus;
}

const vector<Tarefa>& Simulador::getTarefas() const
{
    return tarefas;
}

void Simulador::salvarTickGantt()
{
    vector<int> linha;

    // Cada posição representa uma CPU: salva o ID da tarefa executada
    // ou -1 quando a CPU está ociosa.
    for (const CPU& cpu : cpus)
        linha.push_back(cpu.getTarefaAtual()
                            ? cpu.getTarefaAtual()->getId()
                            : -1);

    // Cada linha corresponde a um tick da simulação.
    historicoGantt.push_back(linha);
}

void Simulador::reiniciar()
{
    // Limpa os registros e os dados da execução anterior.
    historico.clear();
    historicoGantt.clear();
    filaDeProntas.clear();
    cpus.clear();
    tarefas.clear();
    escalonador.reset();
    relogio = Relogio();

    // Recria os componentes e registra o novo estado inicial.
    criaCPUS();
    criaTarefas();
    criaEscalonador();
    atualizaFilaDeProntas();
    salvaHistoricoTick();
}
