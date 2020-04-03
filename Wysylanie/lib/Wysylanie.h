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

    unsigned int CRC(char *addr, int num);

    int Potega2(int x);

    char SumaCRC(int n, int numer);

    int* Binarnie(int liczba);


};


#endif //WYSYLANIE_WYSYLANIE_H
