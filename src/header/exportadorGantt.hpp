#pragma once
#include <string>
#include "simulador.hpp"

// Converte uma cor "RRGGBB" (hexadecimal, com ou sem '#') em componentes 0-255.
// Retorna false se o texto nao for uma cor valida.
// Fica aqui (e nao na interface grafica) para a imagem exportada e a janela do
// simulador usarem exatamente a mesma conversao da cor lida do arquivo de configuracao.
bool corParaRGB(const std::string &hex, int &r, int &g, int &b);

// Gera um arquivo SVG com o grafico de Gantt de TODA a simulacao ate o tick atual
// (requisito 2.4). O desenho e montado a partir do historico do simulador, nao e
// um print da janela, e nao tem limite de tempo: a imagem cresce com a simulacao.
//
// Retorna true em caso de sucesso. Em caso de falha retorna false e preenche
// "erro" com o motivo, para a interface mostrar ao usuario.
bool exportarGanttSVG(const Simulador &simulador, const std::string &caminho, std::string &erro);
