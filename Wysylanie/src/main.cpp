#include <iostream>
#include "../lib/Wysylanie.h"

int main() {
    std::string nazwa;
    std::cout << "Podaj sciezke do pliku ktory chcesz przeslac: \n";
    std::cin >> nazwa;
    Wysylanie wysylanie(nazwa);

    if(!wysylanie.ustawPort()){
        std::cout << "Nie udalo sie ustawic portu\n";
    }
    else
        std::cout << "Udalo sie ustawic port COM1\n";

    wysylanie.wyslaniePliku();


    return 0;
}
