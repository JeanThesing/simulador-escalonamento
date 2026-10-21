#include "header/relogio.hpp"

Relogio::Relogio() {}
Relogio::~Relogio() {}
void Relogio::incrementa()
{
    tick += 1;
}

int Relogio::getTick() const { return tick; }

void Relogio::setTick(int tick) { this->tick = tick; }