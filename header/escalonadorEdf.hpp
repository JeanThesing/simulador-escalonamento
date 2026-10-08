#pragma once
#include <iostream>
#include "escalonador.hpp"

class EscalonadorEDF: public Escalonador{
    private:
        int quantum; // passado como parâmetro da simulacao
    public:
        EscalonadorEDF();
};