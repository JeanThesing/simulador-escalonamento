#pragma once              //evita que o arquivo seja incluído duas vezes
#include <iostream>

// Padrão de projeto singleton (unica instancia em td aplicacao e ponto de acesso global)

class Relogio{
    private:
        int tick = 0; // a cada tick a simulacao deve buscar se precisa fazer alguma acao ou evetos
    public:
        Relogio();
        ~Relogio();
        void incrementa();
        void decrementa();
        int getTick() const;
        //void pausa();
        // implementar loop do relogio aqui?
};