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

char Odbieranie::sumaKontrolna(int flaga){
    char suma = 0;
    if(flaga == 1){ ////// bez CRC
        for(int i=0; i<128; i++) {
            suma += this->blok[i] % 256;
        }
    }
    else{ /////Z CRC

    }
    return suma;
}

bool Odbieranie::odbieraniePliku(int flaga) {
    std::ofstream plik;
    plik.open(nazwa);
    int nrBloku = 0;
    char suma;
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
    WriteFile(uchwyt, &znak, ileZnakow, &rozmiarZnaku, NULL);

    while(ReadFile(uchwyt, &znak, 1, &rozmiarZnaku, NULL) && znak != EOT){

        bool czyOdebrane = false;
        while(!czyOdebrane){
            ReadFile(uchwyt, &nrBloku, 1, &rozmiarZnaku, NULL);
            std::cout <<"Odbieram blok danych nr: " << (int)nrBloku <<'\n';
            ReadFile(uchwyt, &znak, 1, &rozmiarZnaku, NULL);
            if(znak != (char)(255 - nrBloku)){
                std::cout << "Cos poszlo nie tak!!!\n";
            }

            ReadFile(uchwyt, &blok, 128, &rozmiarZnaku, NULL);
            if(flaga == 1 ){
                ReadFile(uchwyt, &suma, 1, &rozmiarZnaku, NULL);
                sumaSprawdzenie = sumaKontrolna(flaga);
            }
            else{

            }

            if(suma == sumaSprawdzenie){
                std::cout << "Udalo sie przeslac pakiet! \n";
              //  for(int i=0; i<128; i++){
              //      if(this->blok[i] != 0){
                       // plik << this->blok[i];
               //     }
             //   }
                plik << this->blok;
                WriteFile(uchwyt, &ACK, 1, &rozmiarZnaku, NULL);
                czyOdebrane = true;

            }
            else{
                std::cout << "Nie udalo sie przeslac pakietu :/\n";
            }

        }

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