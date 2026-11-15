#include "dados.h"

const char *abrev[N] = {"Norte", "Nordeste", "C.Oeste", "Sudeste", "Sul"};

const int ordemFase[N] = {SUDESTE, NORDESTE, CENTRO, SUL, NORTE};

const Color fios[4] = {
    {235, 70, 70, 255}, {70, 140, 255, 255}, {255, 220, 50, 255}, {65, 220, 110, 255}
};

const int vizinho[N][N] = {
    {  0,      1,      1,     0,      0 },
    {  1,      0,      1,     1,      0 },
    {  1,      1,      0,     1,      1 },
    {  0,      1,      1,     0,      1 },
    {  0,      0,      1,     1,      0 }
};

const EvDef evDefs[NUM_EV] = {
    {"Chuvas fortes",      {0.30f, 0.00f,-0.10f, 0.12f, 0.28f}, {0.00f, 0.08f, 0.00f, 0.00f, 0.00f}},
    {"Seca severa",       {-0.40f, 0.10f, 0.00f,-0.25f,-0.30f}, {0.00f, 0.00f, 0.10f, 0.00f, 0.00f}},
    {"Onda de calor",     { 0.00f, 0.12f, 0.30f, 0.00f, 0.00f}, {0.05f, 0.30f, 0.20f, 0.28f, 0.10f}},
    {"Ventos fortes",     { 0.00f, 0.45f, 0.00f, 0.00f, 0.20f}, {0.08f, 0.00f, 0.00f, 0.00f, 0.00f}},
    {"Frente fria",       { 0.00f,-0.15f, 0.00f, 0.00f, 0.00f}, {0.08f, 0.00f, 0.00f, 0.20f, 0.38f}},
    {"Jogo decisivo",     { 0.00f, 0.00f, 0.00f, 0.00f, 0.00f}, {0.00f, 0.20f, 0.00f, 0.40f, 0.10f}},
    {"Manutencao nuclear",{ 0.00f, 0.00f, 0.00f,-0.45f, 0.00f}, {0.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
    {"Sol forte",         { 0.00f, 0.05f, 0.35f, 0.05f, 0.10f}, {0.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
    {"Pico industrial",   { 0.00f, 0.00f, 0.00f, 0.00f, 0.00f}, {0.00f, 0.10f, 0.12f, 0.35f, 0.25f}}
};

const Trilha trilhas[NUM_MUS] = {
    {{57, 53, 48, 55,  57, 53, 48, 52,  50, 53, 48, 55,  57, 53, 50, 52},
     { 0,  1,  1,  2,   0,  1,  1,  0,   0,  1,  1,  2,   0,  1,  0,  2},
     58.0f, 2, 45, 7u, 0.075f},
    {{48, 45, 53, 55,  48, 45, 53, 55,  48, 52, 53, 55,  48, 45, 53, 55},
     { 1,  0,  1,  2,   1,  0,  1,  2,   1,  0,  1,  2,   1,  0,  1,  2},
     66.0f, 2, 40, 11u, 0.065f},
    {{50, 46, 53, 48,  50, 46, 53, 57,  50, 55, 46, 48,  50, 46, 53, 57},
     { 0,  1,  1,  2,   0,  1,  1,  2,   0,  2,  1,  2,   0,  1,  1,  2},
     76.0f, 2, 42, 23u, 0.065f},
    {{52, 48, 55, 50,  52, 48, 55, 59,  52, 57, 48, 50,  52, 48, 50, 59},
     { 0,  1,  1,  2,   0,  1,  1,  2,   0,  0,  1,  2,   0,  1,  2,  2},
     88.0f, 2, 45, 37u, 0.065f},
    {{55, 51, 58, 53,  55, 51, 58, 50,  55, 60, 51, 53,  55, 51, 53, 50},
     { 0,  1,  1,  2,   0,  1,  1,  2,   0,  0,  1,  2,   0,  1,  2,  2},
     100.0f, 2, 48, 51u, 0.06f},
    {{57, 53, 56, 52,  57, 53, 50, 52,  57, 55, 53, 52,  57, 56, 52, 57},
     { 0,  1,  2,  2,   0,  1,  0,  2,   0,  1,  1,  2,   0,  2,  2,  0},
     112.0f, 2, 50, 67u, 0.06f},
    {{53, 55, 50, 57,  53, 55, 52, 57,  53, 57, 50, 55,  53, 55, 52, 53},
     { 1,  2,  0,  0,   1,  2,  0,  0,   1,  0,  0,  2,   1,  2,  0,  1},
     80.0f, 2, 50, 83u, 0.07f}
};

const EstDef estDef[NE] = {
    {"Acre","AC",NORTE,-70.5f,-9.0f},        {"Amazonas","AM",NORTE,-64.5f,-4.0f},
    {"Roraima","RR",NORTE,-61.0f,2.0f},      {"Rondonia","RO",NORTE,-63.0f,-11.0f},
    {"Amapa","AP",NORTE,-52.0f,1.5f},        {"Para","PA",NORTE,-52.5f,-4.5f},
    {"Tocantins","TO",NORTE,-48.3f,-10.2f},
    {"Maranhao","MA",NORDESTE,-45.0f,-5.0f}, {"Piaui","PI",NORDESTE,-42.5f,-7.5f},
    {"Ceara","CE",NORDESTE,-39.5f,-5.2f},    {"Rio G. do Norte","RN",NORDESTE,-36.5f,-5.8f},
    {"Paraiba","PB",NORDESTE,-36.5f,-7.2f},  {"Pernambuco","PE",NORDESTE,-38.0f,-8.4f},
    {"Alagoas","AL",NORDESTE,-36.3f,-9.5f},  {"Sergipe","SE",NORDESTE,-37.3f,-10.7f},
    {"Bahia","BA",NORDESTE,-41.7f,-12.5f},
    {"Mato Grosso","MT",CENTRO,-55.5f,-13.0f},{"Mato G. do Sul","MS",CENTRO,-54.5f,-20.5f},
    {"Goias","GO",CENTRO,-49.5f,-15.8f},     {"Distrito Federal","DF",CENTRO,-47.9f,-15.8f},
    {"Minas Gerais","MG",SUDESTE,-44.5f,-18.5f},{"Espirito Santo","ES",SUDESTE,-40.7f,-19.6f},
    {"Rio de Janeiro","RJ",SUDESTE,-42.8f,-22.2f},{"Sao Paulo","SP",SUDESTE,-48.5f,-22.2f},
    {"Parana","PR",SUL,-51.8f,-24.6f},       {"Santa Catarina","SC",SUL,-50.5f,-27.2f},
    {"Rio G. do Sul","RS",SUL,-53.5f,-29.8f}
};
