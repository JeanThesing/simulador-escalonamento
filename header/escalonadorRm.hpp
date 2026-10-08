#pragma once
#include <iostream>
#include "escalonador.hpp"

class EscalonadorRM: public Escalonador{
    private:
        int quantum; // passado como parâmetro da simulacao
    public:
        EscalonadorRM();
};