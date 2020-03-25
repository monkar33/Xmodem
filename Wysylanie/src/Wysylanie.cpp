//
// Created by Monika on 25.03.2020.
//

#include "../lib/Wysylanie.h"

const char SOH = 01;
const char EOT = 04;
const char ACK = 06;
const char NAK = 15;
const char CAN = 18;
const char C = 43;


char znak;
int licznikZnakow = 1;
unsigned long rozmiarZnaku= sizeof(znak);
static DCB deviceControlBlock;
static HANDLE uchwyt;


char Wysylanie::sumaKontrolna(int flaga) {

    char suma = 0;
    if(flaga == 1){  //-------------------Bez CRC
        for(int i=0; i<128; i++) {
            suma += this->blok[i] % 256;
        }
    }
    else{ //---------------------Z CRC

    }
    return suma;

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
    plik.open(this->nazwa);

    int nrBloku = 0;
    if(plik.good() == true){

        while(!plik.eof()){
            bool czyWyslano = false;
            char suma;
            this->naglowek[0] = SOH;
            this->naglowek[1] = (char)nrBloku;
            this->naglowek[2] = (char)(255 - nrBloku);

            for(int i = 0; i < 128; i++){
                this->blok[i] = 0;
            }
            for(int i =0; i<128  && !plik.eof(); i++){
                this->blok[i] = plik.get();
            }
            do{

                suma = sumaKontrolna(flaga);

                WriteFile(uchwyt, &naglowek[0],licznikZnakow,&rozmiarZnaku, NULL); //SOH
                WriteFile(uchwyt, &naglowek[1],licznikZnakow,&rozmiarZnaku, NULL); // nr bloku/pakietu
                WriteFile(uchwyt, &naglowek[2],licznikZnakow,&rozmiarZnaku, NULL); //255 - nr bloku/pakietu

                WriteFile(uchwyt, &blok, 128, &rozmiarZnaku, NULL);
                WriteFile(uchwyt, &suma, 1, &rozmiarZnaku, NULL);
                ReadFile(uchwyt, &znak, 1, &rozmiarZnaku, NULL);

                if(znak == ACK){
                    czyWyslano = true;
                    std::cout << "Otrzymano ACK, blok danych nr: " << nrBloku <<" zostal wyslany\n";

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
