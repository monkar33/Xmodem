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

    char sumaKontrolna(int flaga);

    bool ustawPort();

    int CRC16(char *addr, int num, int crc);



};


#endif //WYSYLANIE_WYSYLANIE_H
