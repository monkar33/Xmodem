//
// Created by Monika on 25.03.2020.
//

#include <vector>
#include "../lib/Odbieranie.h"

const char SOH = 01;
const char EOT = 04;
const char ACK = 06;
const char NAK = 21;
const char CAN = 18;
const char C = 67;


char znak;
int ileZnakow = 1;
unsigned long rozmiarZnaku= sizeof(znak);
static DCB deviceControlBlock;
static HANDLE uchwyt;



Odbieranie::Odbieranie(const std::string &nazwa) : nazwa(nazwa) {}

Odbieranie::~Odbieranie() {

}
/*
int Odbieranie::CRC(char *dane, int ileZnakow) { // blok danych , ilosc = 128
    uint8_t i;
    uint16_t wCrc = 0xffff;
    while (ileZnakow--) {
        wCrc ^= *(unsigned char *)dane++ << 8;
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1;
    }
    return wCrc & 0xffff;
}

int Odbieranie::Potega2(int x) {
    if( x == 0 ) {
        return 1;
    }
    if( x == 1 ) {
        return 2;
    }
    int wynik = 2;
    for( int i = 2; i <= x; i++ ) {
        wynik = wynik * 2;
    }
    return wynik;
}

char Odbieranie::SumaCRC(int liczba, int ktoryBajt)
{
    int tab[16];
    int reszta = 0;

    for(int i = 0; i < 16; i++) {
        tab[i] = 0;
    }

    for(int i = 0; i < 16; i++) {
        reszta = liczba % 2;
        if (reszta == 1) liczba = (liczba - 1) / 2;
        if (reszta == 0) liczba = liczba / 2;
        tab[15-i] = reszta;
    }

    int koniec;
    int x = 0;
    if(ktoryBajt == 1) koniec = 7;
    if(ktoryBajt == 2) koniec = 15;

    for (int i = 0; i < 8; i++)
        x = x + Potega2(i) * tab[koniec - i];

    for(int i = 0; i< 16; i++) {
        std::cout << tab[i] << " ";
    }
    std::cout << " " << std::endl;
    return (char)x;
}
 */
int Odbieranie::CRC(char *blok, int ileZnakow) {// blok danych , ilosc = 128
    int sumaKontrolnaCRC = 0;

    while (--ileZnakow >= 0)
    {
        sumaKontrolnaCRC = sumaKontrolnaCRC ^ (int)*blok++ << 8; 								 // wez znak i dopisz osiem zer
        for (int i = 0; i < 8; ++i)
            if (sumaKontrolnaCRC & 0x8000) sumaKontrolnaCRC = sumaKontrolnaCRC << 1 ^ 0x1021; // jezli lewy bit == 1 wykonuj XOR generatorm 1021
            else sumaKontrolnaCRC = sumaKontrolnaCRC << 1; 									 // jezli nie to XOR przez 0000, czyli przez to samo
    }
    return (sumaKontrolnaCRC & 0xFFFF);
}

int Odbieranie::Potega2(int x) {
    if( x == 0 ) return 1;
    if( x == 1 ) return 2;

    int wynik = 2;
    for( int i = 2; i <= x; i++ ) wynik = wynik * 2;

    return wynik;
}

char Odbieranie::SumaCRC(int liczba, int ktoryBajt)
{
    int binarna[16];
    int reszta = 0;

    for(int i = 0; i < 16; i++) {
        binarna[i] = 0;
    }

    for(int i = 0; i < 16; i++) {
        reszta = liczba % 2;
        if (reszta == 1) liczba = (liczba - 1) / 2;
        if (reszta == 0) liczba = liczba / 2;
        binarna[15-i] = reszta;
    }

    int koniec;
    int x = 0;
    if(ktoryBajt == 1) koniec = 7;
    if(ktoryBajt == 2) koniec = 15;

    for (int i = 0; i < 8; i++)
        x = x + Potega2(i) * binarna[koniec - i];

    std::cout << "crc bajt: " << ktoryBajt<< ". " << x <<'\n';

    return (char)x;
}

char Odbieranie::sumaKontrolna(){
    char suma = 0;
        for(int i=0; i<128; i++) {
          suma += this->blok[i];
          suma = suma % 256;
        }

    return suma;
}

bool Odbieranie::odbieraniePliku(int flaga) {
    std::ofstream plik;
    plik.open(nazwa, std::ios::binary);
    int nrBloku = 0;
    char sumaSprawdzenie;

    if(flaga == 1)
        znak = NAK;
    else if(flaga == 2)
        znak = C;
    else {
        std::cout <<"Niepoprawny Wybor\n";
        return 0;
    }

    std::cout <<"Wysylam  NAK/C\n";
    for(int i = 0; i < 20; i++){

        WriteFile(uchwyt, &znak, ileZnakow, &rozmiarZnaku, NULL);
        ReadFile(uchwyt, &znak, 1, &rozmiarZnaku, NULL);
        std::cout <<" znak " << znak << std::endl;
        if( znak == SOH) {
            break;
        }
        sleep(3000);
    }

    while(znak == SOH){
        bool czyOdebrane = false;
        bool poprawnaSuma = false;
        while(!czyOdebrane){
            ReadFile(uchwyt, &nrBloku, 1, &rozmiarZnaku, NULL);
            std::cout <<"Odbieram blok danych nr: " << (int)nrBloku <<'\n';
            ReadFile(uchwyt, &znak, 1, &rozmiarZnaku, NULL);
            if(znak != (char)(255 - nrBloku)){
                std::cout << "Numer pakietu nie jest poprawny!!!\n";
            }
            for(int i =0; i < 128; i++){
                ReadFile(uchwyt, &blok[i], 1, &rozmiarZnaku, NULL);
            }
            if(flaga == 1 ){
                char suma;
                ReadFile(uchwyt, &suma, 1, &rozmiarZnaku, NULL);
                sumaSprawdzenie = sumaKontrolna();
                if(suma == sumaSprawdzenie){
                    poprawnaSuma = true;
                }
            }
            else{
                char suma[2];
                char sumaCrcSprawdzenie[2];
                ReadFile(uchwyt, &suma, 2, &rozmiarZnaku, NULL);
                int crc = CRC(blok,128);
                std::cout << "crc: " <<crc << '\n';
                sumaCrcSprawdzenie[0] = SumaCRC(crc,1);
                sumaCrcSprawdzenie[1] = SumaCRC(crc,2);

                std::cout << "Suma odebrana: " << (int)suma[0]<<"  " << (int)suma[1] << "\n Suma obliczona: " << (int)sumaCrcSprawdzenie[0] << " "<< (int)sumaCrcSprawdzenie[1] << "\n";

                if(suma[0] == sumaCrcSprawdzenie[0] && suma[1] == sumaCrcSprawdzenie[1]){
                    poprawnaSuma = true;
                }
            }

            if(poprawnaSuma) {
                std::cout << "Udalo sie przeslac pakiet! \n";
                int ile = 128;
                if(blok[127] == '0' and blok[126] == '0' and blok[125] == '0') {
                    for (int i = 127; i >= 0 and this->blok[i] == '0'; i--) {
                        ile--;
                    }
                }
                std::vector<char> buforPliku;
                for(int i = 0 ; i < ile; i++){
                    buforPliku.push_back(blok[i]);
                }

                plik.write(buforPliku.data(), buforPliku.size());

                WriteFile(uchwyt, &ACK, 1, &rozmiarZnaku, NULL);
                czyOdebrane = true;

                }

            else{
                std::cout << "Nie udalo sie przeslac pakietu :/\n"
                             "Przesylam NAK\n";
                WriteFile(uchwyt, &NAK, 1, &rozmiarZnaku, NULL);
            }

        }

        ReadFile(uchwyt, &znak, 1, &rozmiarZnaku, NULL);

    }
    if(znak == EOT){
        std::cout << "Odebrano EOT, transmisja zakonczona sukcesem! \n";
        WriteFile(uchwyt, &ACK, 1, &rozmiarZnaku, NULL);
        return true;
    }
    if(znak == CAN) {
        std::cout << "Polaczenie przerwane.\n";
        return false;
    }

    plik.close();
}


bool Odbieranie::ustawPort() {
    uchwyt = CreateFile("COM2", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (uchwyt != INVALID_HANDLE_VALUE) {
        deviceControlBlock.DCBlength = sizeof(deviceControlBlock);
        deviceControlBlock.BaudRate = CBR_9600; // prędkość transmisji, CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400, CBR_4800, CBR_9600
        deviceControlBlock.fParity = TRUE; //TRUE jeśli ustawiamy kolejną opcję dla sprawdzenia parzystości
        deviceControlBlock.Parity = NOPARITY; //Ustawienie parzystości: EVENPARITY (parzysta), MARKPARITY (bit parzystości 1), NOPARITY (bez bitu parzystości), ODDPARITY (nieparzysta)
        deviceControlBlock.StopBits = ONESTOPBIT; // Ustawienie bitu stopu: ONESTOPBIT (jeden bit), TWOSTOPBITS (dwa bity)
        deviceControlBlock.ByteSize = 8; //Liczba bitów wysyłanych/odebranych danych (UWAGA wartościod 5 do 8)

        deviceControlBlock.fDtrControl = DTR_CONTROL_DISABLE; //Kontrola linii DTR: DTR_CONTROL_DISABLE (sygnał nieaktywny), DTR_CONTROL_ENABLE (sygnał aktywny), DTR_CONTROL_HANDSHAKE
        deviceControlBlock.fRtsControl = RTS_CONTROL_DISABLE; //Kontrola linii RTS: RTS_CONTROL_DISABLE (sygnał nieaktywny), RTS_CONTROL_ENABLE (sygnał aktywny), RTS_CONTROL_HANDSHAKE
        deviceControlBlock.fOutxCtsFlow = FALSE;
        deviceControlBlock.fOutxDsrFlow = FALSE;
        deviceControlBlock.fDsrSensitivity = FALSE;
        deviceControlBlock.fAbortOnError = FALSE;
        deviceControlBlock.fOutX = FALSE;
        deviceControlBlock.fInX = FALSE;
        deviceControlBlock.fErrorChar = FALSE;
        deviceControlBlock.fNull = FALSE;
        SetCommState(uchwyt, &deviceControlBlock);

        return true;
    }
    else return false;
}