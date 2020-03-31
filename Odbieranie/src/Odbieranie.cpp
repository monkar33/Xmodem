//
// Created by Monika on 25.03.2020.
//

#include "../lib/Odbieranie.h"

const char SOH = 01;
const char EOT = 04;
const char ACK = 06;
const char NAK = 15;
const char CAN = 18;
const char C = 43;


char znak;
int ileZnakow = 1;
unsigned long rozmiarZnaku= sizeof(znak);
static DCB deviceControlBlock;
static HANDLE uchwyt;



Odbieranie::Odbieranie(const std::string &nazwa) : nazwa(nazwa) {}



Odbieranie::~Odbieranie() {

}

int Odbieranie::CRC(char *dane, int ileZnakow) { // blok danych , ilosc = 128
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

    return (char)x;
}

char Odbieranie::sumaKontrolna(){
    char suma = 0;
        for(int i=0; i<128; i++) {
            suma = suma ^ this->blok[i];
        }

    return suma;
}

bool Odbieranie::odbieraniePliku(int flaga) {
    std::ofstream plik;
    plik.open(nazwa);
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
        std::cout << (int)znak << std::endl;
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

            //ReadFile(uchwyt, &blok, 128, &rozmiarZnaku, NULL);
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
                char sumaSprawdzenie[2];
                ReadFile(uchwyt, &suma, 2, &rozmiarZnaku, NULL);
                char crc = CRC(blok,128);
                sumaSprawdzenie[0] = SumaCRC(crc,1);
                sumaSprawdzenie[1] = SumaCRC(crc,2);
                if(suma[0] == sumaSprawdzenie[0] && suma[1] == sumaSprawdzenie[1]){
                    poprawnaSuma = true;
                }
            }

            if(poprawnaSuma) {
                std::cout << "Udalo sie przeslac pakiet! \n";
                int ile = 128;
                if(blok[127] == '#' and blok[126] == '#' and blok[125] == '#') {
                    for (int i = 127; i >= 0 and this->blok[i] == '#'; i--) {
                        ile--;
                    }
                }
                plik.write(this->blok, ile);

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