#include "header/registroEscalonadores.hpp"
#include "header/escalonadorRm.hpp"
#include "header/escalonadorEdf.hpp"
#include <algorithm>
#include <cctype>
#include <map>

namespace
{
    // Converte para maiusculas (chave do registro).
    std::string maiusculas(std::string texto)
    {
        std::transform(texto.begin(), texto.end(), texto.begin(),
                       [](unsigned char c)
                       { return static_cast<char>(std::toupper(c)); });
        return texto;
    }

    // O mapa e uma variavel local estatica de funcao (e nao global) para garantir que ele ja
    // exista quando um REGISTRAR_ESCALONADOR de outro .cpp rodar durante a inicializacao do
    // programa (a ordem de inicializacao de globais entre arquivos diferentes nao e definida).
    // std::map mantem os nomes em ordem alfabetica, entao o combo da interface fica estavel.
    std::map<std::string, FabricaEscalonador> &registro()
    {
        static std::map<std::string, FabricaEscalonador> mapa;
        return mapa;
    }
}

void registrarEscalonador(const std::string &nome, FabricaEscalonador fabrica)
{
    registro()[maiusculas(nome)] = std::move(fabrica);
}

bool algoritmoRegistrado(const std::string &nome)
{
    return registro().count(maiusculas(nome)) > 0;
}

std::unique_ptr<Escalonador> criarEscalonador(const std::string &nome)
{
    auto it = registro().find(maiusculas(nome));
    if (it == registro().end())
        return nullptr;
    return it->second();
}

std::vector<std::string> nomesAlgoritmos()
{
    std::vector<std::string> nomes;
    for (const auto &par : registro())
        nomes.push_back(par.first);
    return nomes;
}

// Algoritmos que acompanham o simulador (Projeto A): RM e EDF.
// Sao registrados aqui para que existam mesmo sem nenhum outro arquivo se registrar.
REGISTRAR_ESCALONADOR("RM", EscalonadorRM);
REGISTRAR_ESCALONADOR("EDF", EscalonadorEDF);
