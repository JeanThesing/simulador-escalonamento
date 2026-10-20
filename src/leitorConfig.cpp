#include "header/leitorConfig.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <set>

// funcoes auxiliares
// remove espacos/tabs/CR no inicio e no fim
// Remove espaços e quebras de linha do início e do fim da string
static std::string removerEspacos(const std::string &texto)
{
    // encontra o primeiro caractere que não e espaço
    size_t inicio = texto.find_first_not_of(" \t\r\n");

    // se a string estiver vazia ou tiver apenas espaços
    if (inicio == std::string::npos)
        return "";

    // Encontra o último caractere que não é espaço
    size_t fim = texto.find_last_not_of(" \t\r\n");

    // Retorna a string sem os espaços das extremidades
    return texto.substr(inicio, fim - inicio + 1);
}

// converte para maiusculo (strings do arquivo nao diferenciam maiusculas/minusculas - req. 3.3.2)
static std::string converterMaiusculas(std::string texto)
{
    // percorre cada caractere e converte para maiusculo
    std::transform(texto.begin(), texto.end(), texto.begin(),
                   [](unsigned char c)
                   { return std::toupper(c); });

    // retorna string convetrtida
    return texto;
}

// divide uma linha em campos separados por ';'
static std::vector<std::string> dividirCampos(const std::string &linha)
{
    std::vector<std::string> campos;
    std::string campoAtual;

    // percorre cada caractere da linha
    for (char caractere : linha)
    {
        if (caractere == ';')
        {
            // salva o campo sem espaços nas extremidades
            campos.push_back(removerEspacos(campoAtual));
            campoAtual.clear(); // limpa para receber o próximo campo
        }
        else
        {
            campoAtual += caractere;
        }
    }

    // adiciona o ultimo campo
    campos.push_back(removerEspacos(campoAtual));

    // remove campo vazio caso a linha termine com ;
    if (campos.size() > 1 && campos.back().empty())
        campos.pop_back();

    return campos;
}

// converte texto em int, verificando se o valor é válido
#include <charconv>
#include <system_error>

// Converte uma string para inteiro
static bool lerInt(const std::string &texto, int &resultado)
{
    if (texto.empty())
        return false;
    // tenta converter o texto para inteiro
    auto conversao = std::from_chars(
        texto.data(),
        texto.data() + texto.size(),
        resultado
    );
    // verifica se o número e valido e se não sobraram caracteres
    return conversao.ec == std::errc() &&
           conversao.ptr == texto.data() + texto.size();
}

// Verifica se a cor é hexadecimal e possui 6 dígitos
static bool validarCor(std::string &cor)
{
    // remove # do início caso tenhas
    if (!cor.empty() && cor[0] == '#')
        cor.erase(0, 1);
    // ve se a cor tem 6 digitos
    if (cor.size() != 6)
        return false;
    // Verifica se todos os caracteres são hexadecimais
    for (char caractere : cor)
    {
        if (!std::isxdigit(static_cast<unsigned char>(caractere)))
            return false;
    }
    // converte letra pra maisuclo
    cor = converterMaiusculas(cor);
    return true;
}

// LEITURA DO ARQUVIO
// adiciona uma msg informando a linha do erro
void registrarErro(Configuracao &config, int linha, const std::string &mensagem)
{
    config.erros.push_back("Linha " + std::to_string(linha) + ": " + mensagem);
}

// add um aviso
void registrarAviso(Configuracao &config, int linha, const std::string &mensagem)
{
    config.avisos.push_back("Linha " + std::to_string(linha) + ": " + mensagem);
}

// le o algoritmo, quantum e qntd de CPU
void lerCabecalho(const std::vector<std::string> &campos,
                  Configuracao &config, int linha)
{
    if (campos.size() > 3)
        registrarErro(config, linha, "Cabecalho com mais de 3 campos");
    // algoritmo
    if (campos.size() >= 1 && !campos[0].empty())
    {
        std::string algoritmo = converterMaiusculas(campos[0]);

        if (algoritmo == "RM" || algoritmo == "EDF")
            config.algoritmo = algoritmo;
        else
            registrarErro(config, linha, "Algoritmo invalido");
    }

    // Quantum
    if (campos.size() >= 2 && !campos[1].empty())
    {
        int quantum;

        if (lerInt(campos[1], quantum) && quantum > 0)
            config.quantum = quantum;
        else
            registrarErro(config, linha, "Quantum invalido");
    }

    // Quantidade de CPUs
    if (campos.size() >= 3 && !campos[2].empty())
    {
        int quantidade;

        if (lerInt(campos[2], quantidade) && quantidade > 0)
            config.qtdeCpus = quantidade;
        else
            registrarErro(config, linha, "Quantidade de CPUs invalida");
    }
}

// LEITURA E VALIDACAO DAS TAREFAS
void lerTarefa(const std::vector<std::string> &campos,
               Configuracao &config,
               int linha,
               std::set<int> &idsUsados,
               std::set<std::string> &coresUsadas)
{
    // verificacao da qnt de campos: 6 ou 7 (7 se tiver comentario)
    if (campos.size() < 6 || campos.size() > 7)
    {
        registrarErro(config, linha, "Quantidade de campos da tarefa invalida");
        return;
    }

    int id, ingresso, duracao, periodo, prazo;
    std::string cor = campos[1];

    bool valido = true;

    // validacao do id
    if (!lerInt(campos[0], id) || id <= 0)
    {
        registrarErro(config, linha, "ID invalido");
        valido = false;
    }

    // validacao da cor
    if (!validarCor(cor))
    {
        registrarErro(config, linha, "Cor invalida");
        valido = false;
    }

    // validacao do ingreso
    if (!lerInt(campos[2], ingresso) || ingresso < 0)
    {
        registrarErro(config, linha, "Ingresso invalido");
        valido = false;
    }

    // validacao da duracao
    if (!lerInt(campos[3], duracao) || duracao <= 0)
    {
        registrarErro(config, linha, "Duracao invalida");
        valido = false;
    }

    // valida o peridoo
    if (!lerInt(campos[4], periodo) || periodo < 0)
    {
        registrarErro(config, linha, "Periodo invalido");
        valido = false;
    }

    // validacao de prazo (tem q ser maior que 0, pq o prazo 0 significa q a tarefa tem q terminar no mesmo tick que foi ativada, oq n faz sentido)
    if (!lerInt(campos[5], prazo) || prazo <= 0)
    {
        registrarErro(config, linha, "Prazo invalido");
        valido = false;
    }

    // Verifica se os dados são válidos
    if (!valido)
        return;

    // Verifica se o ID já foi utilizado
    if (idsUsados.count(id) > 0)
    {
        registrarErro(config, linha, "ID repetido");
        return;
    }

    // ignora tarefas aperiódicas no Projeto A
    if (periodo == 0)
    {
        registrarAviso(config, linha, "Tarefa aperiodica ignorada");
        return;
    }

    // Verifica se a cor já foi utilizada
    if (coresUsadas.count(cor) > 0)
        registrarAviso(config, linha, "Cor repetida");

    // Registra o ID e a cor
    idsUsados.insert(id);
    coresUsadas.insert(cor);

    // Adiciona a tarefa ao simulador
    config.tarefas.emplace_back(id, cor, ingresso, duracao, periodo, prazo);
}

/*====================================================================================================================== CARREGAR CONIFGURACAO=====================*/

// carrega a config e as tarefas de um arquivo
bool carregarConfiguracao(const std::string &caminho, Configuracao &config)
{
    // reinicia config caso ja tenha sido carregada antes
    config = Configuracao();

    // abre o arquvio no caminho especificado
    std::ifstream arquivo(caminho);

    if (!arquivo.is_open())
    {
        config.erros.push_back("Nao foi possivel abrir o arquivo: " + caminho);
        return false;
    }

    std::set<int> idsUsados;
    std::set<std::string> coresUsadas;

    std::string linha;
    int numeroLinha = 0;
    bool cabecalhoLido = false;

    // percorre as linhas do arquivo
    while (std::getline(arquivo, linha))
    {
        numeroLinha++;

        // Remove o BOM de arquivos UTF-8
        if (numeroLinha == 1 && linha.size() >= 3 &&
            linha.compare(0, 3, "\xEF\xBB\xBF") == 0)
        {
            linha.erase(0, 3);
        }

        // remove espacos desnecessarios
        linha = removerEspacos(linha);
        // ignora linhas vazias
        if (linha.empty())
            continue;
        // separa campos pelo ;
        std::vector<std::string> campos = dividirCampos(linha);

        // le o cabeçalho na primeira linha valida
        if (!cabecalhoLido)
        {
            lerCabecalho(campos, config, numeroLinha);
            cabecalhoLido = true;
        }
        else
        {
            // le tarefas nas outras linhas
            lerTarefa(campos, config, numeroLinha, idsUsados, coresUsadas);
        }
    }

    // verifica se o arquivo tinha um cabecalho
    if (!cabecalhoLido)
        registrarErro(config, 0, "Arquivo vazio");

    // verifica se tem tarefas periodicas validas
    else if (config.tarefas.empty())
        config.erros.push_back("Nenhuma tarefa periodica valida encontrada");

    // retorna true se n tiver erros, false se tiver erros
    return config.erros.empty();
}