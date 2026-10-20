#include "header/relogio.hpp"

Relogio::Relogio() {}
Relogio::~Relogio() {}
void Relogio::incrementa()
{
    tick += 1;
}
void Relogio::decrementa()
{
    if (tick > 0)
    {
        tick -= 1;
    }
}

int Relogio::getTick() const{ return tick; }
