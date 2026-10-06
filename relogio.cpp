#include "relogio.hpp"

Relogio::Relogio(){}
Relogio::~Relogio(){}
void Relogio::incrementa(){
    tick+=taxa;
}
void Relogio::decrementa(){
    if(tick>=taxa){
        tick-=taxa;
    }
}