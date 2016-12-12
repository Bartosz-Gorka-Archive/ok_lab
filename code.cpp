/* OPTYMALIZACJA KOMBINATORYCZNA
 * Temat: Metaheurystyki w problemie szeregowania zada�
 * Autor: Bartosz G�rka, Mateusz Kruszyna
 * Data: Listopad 2016r.
*/

// Biblioteki u�ywane w programie
#include <iostream> // I/O stream
#include <vector> // Vector table
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream> 

using namespace std;

// Struktura danych w pami�ci
struct Task {
	int assigment; // Przydział zadania pierwszego do maszyny x = [0, 1]
	int durationFirstPart; // Czas cz�ci 1 zadania
	int durationSecondPart; // Czas cz�ci 2 zadania
	int timeEndFirstPart; // Moment zako�czenia cz�ci 1
	int timeEndSecondPart; // Moment zako�czenia cz�ci 2
};

struct Maintenance {
	int assigment; // Numer maszyny
	int readyTime; // Czas gotowo�ci (pojawienia si�)
	int duration; // Czas trwania przerwania
};

// Generator przestoj�ww na maszynie
void GeneratorPrzestojow(vector<Maintenance*> &lista, int liczbaPrzerwanFirstProcessor, int liczbaPrzerwanSecondProcessor, int lowerTimeLimit, int upperTimeLimit, int lowerReadyTime, int upperReadyTime) {
	srand(time(NULL));
	
	int size = (upperReadyTime - lowerReadyTime) + (upperTimeLimit - lowerTimeLimit);
	bool * maintenanceTimeTable = new bool[size] {}; // Jedna tablica bo przerwania na maszynach nie mog� si� nak�ada� na siebie 
	
	int liczbaPrzerwan = liczbaPrzerwanFirstProcessor + liczbaPrzerwanSecondProcessor;
	
	for(int i = 0; i < liczbaPrzerwan; i++) {
		Maintenance * przerwa = new Maintenance;
		
		// Losowanie przerwy na kt�r� maszyn� ma trafi�
		if(liczbaPrzerwanFirstProcessor == 0) {
			przerwa->assigment = 1;
			liczbaPrzerwanSecondProcessor--;
		} else if (liczbaPrzerwanSecondProcessor == 0){
			przerwa->assigment = 0;
			liczbaPrzerwanFirstProcessor--;
		} else {
			przerwa->assigment = rand() % 2;
			if(przerwa->assigment == 0)
				liczbaPrzerwanFirstProcessor--;
			else
				liczbaPrzerwanSecondProcessor--;
		}
		
		// Randomowy czas trwania
			int duration = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			przerwa->duration = duration;
			
		// Random punkt startu + sprawdzenie czy jest to mo�liwe
			int readyTime = 0;
			int startTimeCheck, stopTimeCheck = 0;
			
			while(true) {
				readyTime = lowerReadyTime + (int)(rand() / (RAND_MAX + 1.0) * upperReadyTime);
				
				startTimeCheck = readyTime - lowerReadyTime;
				stopTimeCheck = startTimeCheck + duration;
				// Sprawdzenie czy mo�na da� przerwanie od readyTime
					bool repeatCheck = false;
					for(int j = startTimeCheck; j < stopTimeCheck; j++) {
						if(maintenanceTimeTable[j]) {
							repeatCheck = true;
							break; // Konieczne jest ponowne losowanie czasu rozpocz�cia
						}
					}
					
					if(!repeatCheck) {
						break; // Mo�na opu�ci� p�tle while - znaleziono konfiguracj� dla przerwania
					}
			}
			
			// Zapis przerwania w tablicy pomocniczej
				for(int j = startTimeCheck; j < stopTimeCheck; j++) {
					maintenanceTimeTable[j] = true;
				}
			
			// Uzupe�nienie danych o przerwaniu
				przerwa->readyTime = readyTime;
				przerwa->duration = duration;
			
			// Dodanie przestoju do listy
				lista.push_back(przerwa);
	}
}

// Generator instancji problemu
void GeneratorInstancji(vector<Task*> &lista, int n, int lowerTimeLimit, int upperTimeLimit) {
	srand(time(NULL));
	
	for(int i = 0; i < n; i++) {
		Task * zadanie = new Task;
		
		// Przydzia� maszyny do cz�ci 1 zadania
			zadanie->assigment = rand() % 2;
		
		// Randomy na czas trwania zadania
			zadanie->durationFirstPart = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			zadanie->durationSecondPart = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			
		// Czas zako�czenia p�ki co ustawiony na 0 = zadanie nie by�o u�ywane
			zadanie->timeEndFirstPart = 0;
			zadanie->timeEndSecondPart = 0;
		
		// Dodanie zadania do listy
			lista.push_back(zadanie);	
	}
}

// Zapis instancji do pliku
void ZapiszInstancjeDoPliku(vector<Task*> &lista, int numer_instancji_problemu) {
	ofstream file;
	file.open("instancje.txt");	
	
	if(file.is_open()) {
		file << "**** " << numer_instancji_problemu << "  ****" << endl;
		
		// Uzupe�nienie pliku o wygenerowane czasy pracy
			int ilosc_zadan = lista.size();
			for(int i = 0; i< ilosc_zadan; i++) {
				file << lista[i]->durationFirstPart << ":" << lista[i]->durationSecondPart << ":"
				<< lista[i]->assigment << ":" << 1 - lista[i]->assigment << ";" << endl;
			}
		
		// Uzupe�nienie pliku o czasy przestoj�w maszyn
	}	
	
	file.close();
}

void OdczytPrzerwan(vector<Maintenance*> &listaPrzerwan) {
	int size = listaPrzerwan.size();
	for(int i = 0; i < size; i++) {
		cout << "Maszyna = " << listaPrzerwan[i]->assigment << " | Start = " << listaPrzerwan[i]->readyTime << " | Czas trwania = " << listaPrzerwan[i]->duration << endl;
	}
}

int main() {
	int rozmiar_instancji = 50;
	int numer_instancji_problemu = 1;
	
	// Utworzenie wektora na n zada�
		vector<Task*> lista_zadan;
	
	// Wektor przerwa� pracy na maszynach
		vector<Maintenance*> listaPrzerwan; 
	
	// Wygenerowanie zada�
		GeneratorInstancji(lista_zadan, rozmiar_instancji, 5, 15);
		
	// Wygenerowanie przerwa�	
		GeneratorPrzestojow(listaPrzerwan, 2, 2, 0, 10, 0, 25);
		OdczytPrzerwan(listaPrzerwan);
		
	// Zapis danych do pliku
		ZapiszInstancjeDoPliku(lista_zadan, numer_instancji_problemu);

	return 0;
}
