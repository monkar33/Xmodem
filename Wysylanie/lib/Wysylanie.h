//
// Created by Monika on 25.03.2020.
//

#ifndef WYSYLANIE_WYSYLANIE_H
#define WYSYLANIE_WYSYLANIE_H


#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>



class Wysylanie {
    char blok[128];
    char naglowek[3];
    std::string nazwa;


public:
    Wysylanie(const std::string &nazwa);

    virtual ~Wysylanie();

    bool wyslaniePliku();

    char sumaKontrolna();

    bool ustawPort();

    int CRC(char *blok, int ileZnakow);

    int Potega2(int x);

    char SumaCRC(int n, int numer);



};


#endif //WYSYLANIE_WYSYLANIE_H
