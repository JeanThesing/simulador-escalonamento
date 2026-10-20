#include "header/simulador.hpp"
using namespace std;

int main()
{
    Simulador simulador(15, 5, "RM");
    simulador.criaCPUS();
    simulador.criaTarefas();
    return 0;
}