//
// Created by Monika on 25.03.2020.
//

#ifndef ODBIERANIE_ODBIERANIE_H
#define ODBIERANIE_ODBIERANIE_H


#include <string>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <unistd.h>

class Odbieranie {
    char blok[128];
    char naglowek[3];
    std::string nazwa;


public:
    Odbieranie(const std::string &nazwa);

    virtual ~Odbieranie();

    bool odbieraniePliku(int flaga);

    char sumaKontrolna();

    bool ustawPort();

    int CRC(char *blok, int ileZnakow);

    int Potega2(int x);

    char SumaCRC(int n, int numer);

};


#endif //ODBIERANIE_ODBIERANIE_H
