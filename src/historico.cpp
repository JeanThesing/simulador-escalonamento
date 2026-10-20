#include "header/historico.hpp"

void Historico::salvar(const Snapshot& snapshot)
{
    snapshots.push_back(snapshot);
}

const Snapshot* Historico::buscar(int tick) const
{
    for (const Snapshot& snapshot : snapshots)
    {
        if (snapshot.tick == tick)
            return &snapshot;
    }

    return nullptr;
}

void Historico::limpar()
{
    snapshots.clear(); //limpa o vetor de snapshots
}