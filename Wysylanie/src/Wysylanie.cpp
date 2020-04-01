//
// Created by Monika on 25.03.2020.
//

#include <vector>
#include "../lib/Wysylanie.h"

const char SOH = 01;
const char EOT = 04;
const char ACK = 06;
const char NAK = 21;
const char CAN = 18;
const char C = 67;


char znak;
int licznikZnakow = 1;
unsigned long rozmiarZnaku= sizeof(znak);
static DCB deviceControlBlock;
static HANDLE uchwyt;

char Wysylanie::sumaKontrolna() {
    char suma = 0;
    for(int i=0; i<128; i++) {
        //  suma = suma ^ this->blok[i];
        suma += this->blok[i];
        suma = suma % 256;
    }

    return suma;
}

int Wysylanie::CRC(char *dane, int ileZnakow) { // blok danych , ilosc = 128
    char i;
    int CRC = 0;
    while (--ileZnakow >= 0)
    {
        CRC = CRC ^ (int) *dane++ << 8;
        i = 8;
        do{
            if (CRC & 0x8000)
                CRC = CRC << 1 ^ 0x1021;
            else
                CRC = CRC << 1;
        } while(--i);
    }
    return (CRC);
}

int Wysylanie::Potega2(int x) {
    if( x == 0 ) return 1;
    if( x == 1 ) return 2;

    int wynik = 2;
    for( int i = 2; i <= x; i++ ) wynik = wynik * 2;

    return wynik;
}

char Wysylanie::SumaCRC(int liczba, int ktoryBajt)
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

    return (char)x;
}

bool Wysylanie::wyslaniePliku(){

    int flaga;
    std::cout << "Oczekiwanie na znak....\n";
    ReadFile(uchwyt, &znak, licznikZnakow, &rozmiarZnaku, NULL);

    if(znak == NAK){
        std::cout << "Otrzymano znak NAK: " << znak <<'\n';
        flaga = 1;
    }
    else if( znak == C){
        std::cout << "Otrzymano znak C" << znak <<'\n';
        flaga = 2;
    }

    std::ifstream plik;
    plik.open(this->nazwa,std::fstream::binary);


    int nrBloku = 0;
    if(plik.good()){

        std::vector<char> buforPliku;
        int rozmiarPliku = 0;
        while(plik){
            char b = plik.get();
            if(plik){
                buforPliku.push_back(b);
                rozmiarPliku ++;
            }

        }
        plik.close();
        int iter = 0;
        while(iter <= rozmiarPliku){
            bool czyWyslano = false;
            this->naglowek[0] = SOH;
            this->naglowek[1] = (char)nrBloku;
            this->naglowek[2] = (char)(255 - nrBloku);

            for(int i=0; i<128; i++){
                this->blok[i] =buforPliku[i+iter];
                if(i+iter > rozmiarPliku){
                    this->blok[i] = 0x23;
                }
            }
            iter +=128;

            do{
                WriteFile(uchwyt, &naglowek[0],licznikZnakow,&rozmiarZnaku, NULL); //SOH
                WriteFile(uchwyt, &naglowek[1],licznikZnakow,&rozmiarZnaku, NULL); // nr bloku/pakietu
                WriteFile(uchwyt, &naglowek[2],licznikZnakow,&rozmiarZnaku, NULL); //255 - nr bloku/pakietu

                for(int i =0; i < 128; i++){
                    WriteFile(uchwyt, &blok[i], 1, &rozmiarZnaku, NULL);
                }
                if(flaga == 1){
                    char suma;
                    suma = sumaKontrolna();
                    WriteFile(uchwyt, &suma, 1, &rozmiarZnaku, NULL);
                }
                else if(flaga == 2){
                    char crc = CRC(blok,128);
                    char suma[2];
                    suma[0] = SumaCRC(crc, 1);
                    suma[1] = SumaCRC(crc, 2);
                    WriteFile(uchwyt, &suma, 2, &rozmiarZnaku, NULL);
                }

                ReadFile(uchwyt, &znak, 1, &rozmiarZnaku, NULL);

                if(znak == ACK){
                    czyWyslano = true;
                    std::cout << "Otrzymano ACK, blok danych nr: " << nrBloku <<" zostal wyslany\n";

                }
                if(znak == NAK){
                    std::cout << "Pakiet nie zostal wyslany poprawnie. Ponowienie proby........\n";
                }

            }while(!czyWyslano);

            nrBloku++;
            if(nrBloku == 256)
                nrBloku = 0;



        }


        WriteFile(uchwyt, &EOT, 1, &rozmiarZnaku, NULL);
        std::cout << "Wyslano EOT\n";
        ReadFile(uchwyt, &znak, 1, &rozmiarZnaku, NULL);
        if(znak == ACK){
            std::cout << "Przesylanie zakonczone" << std::endl;
        }


    }
    else{
        std::cout << "Nie odnaleziono pliku\n";

        CloseHandle(uchwyt);
        return false;
    }

    CloseHandle(uchwyt);
    return true;

}

Wysylanie::Wysylanie(const std::string &nazwa) : nazwa(nazwa) {}

Wysylanie::~Wysylanie() {

}

bool Wysylanie::ustawPort(){
    uchwyt = CreateFile("COM1", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
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

