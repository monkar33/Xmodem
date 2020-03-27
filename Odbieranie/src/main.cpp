#include <iostream>
#include "../lib/Odbieranie.h"


int main() {
    std::string nazwa;
    std::cout << "Podaj nazwe pliku do zapisu: \n";
    std::cin >> nazwa;
   // nazwa +=".txt";

    Odbieranie odbieranie(nazwa);
    if(!odbieranie.ustawPort()){
        std::cout << "Nie udalo sie ustawic portu\n";
    }
    else
        std::cout << "Udalo sie ustawic port COM2\n";

    int flaga;
    std::cout << "Wybierz wersje protokolu XMODEM ktora chcesz wykorzystac: \n"
                 "1) Bez CRC\n"
                 "2) Z CRC\n";
    std::cin >> flaga;

    odbieranie.odbieraniePliku(flaga);


}
