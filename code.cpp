/* OPTYMALIZACJA KOMBINATORYCZNA
 * Temat: Metaheurystyki w problemie szeregowania zadań
 * Autor: Bartosz Górka, Mateusz Kruszyna
 * Data: Grudzień 2016r.
*/
// TEST 002
// Biblioteki używane w programie
#include <iostream> // I/O stream
#include <vector> // Vector table
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream> 
#include <climits> // INT_MAX do generatora losowego
#include <algorithm> // Sortowanie przerwań

using namespace std;

// DEFINICJE GLOBALNE
#define DEBUG false // TRYB DEBUGOWANIA [true / false]
#define MIN_TASK_COUNTER 30 // PO ilu iteracjach sprawdzać zapętlenie [Wartość liczbowa > 0]

#define LOWER_TIME_TASK_LIMIT 5 // Dolne ograniczenie długości zadania [Wartość liczbowa > 0]
#define UPPER_TIME_TASK_LIMIT 15 // Górne ograniczenie długości zadania [Wartość liczbowa > 0]

#define MAINTENANCE_FIRST_PROCESSOR 3 // Ilość przerwań na pierwszej maszynie [Wartość liczbowa >= 0]
#define MAINTENANCE_SECOND_PROCESSOR 3 // Ilość przerwań na drugiej maszynie [Wartość liczbowa >= 0]

#define LOWER_TIME_MAINTENANCE_LIMIT 5 // Dolne ograniczenie długości przerwania [Wartość liczbowa >= 0]
#define UPPER_TIME_MAINTENANCE_LIMIT 20 // Górne ograniczenie długości przerwania [Wartość liczbowa > 0]

#define LOWER_READY_TIME_MAINTENANCE_LIMIT 0 // Dolne ograniczenie czasu gotowości przerwania [Wartość liczbowa > 0]
#define UPPER_READY_TIME_MAINTENANCE_LIMIT 200 // Górne ograniczenie czasu gotowości przerwania [Wartość liczbowa > 0]

#define INSTANCE_SIZE 15 // Rozmiar instancji problemu
#define INSTANCE_NUMBER 1 // Numer instancji problemu (może być zmieniana przy odczycie danych z pliku)

ofstream debugFile; // Zmienna globalna używana przy DEBUG mode

// Struktura danych w pamięci
struct Task {
	int assigment; // PrzydziaĹ‚ zadania pierwszego do maszyny x = [0, 1]
	int durationFirstPart; // Czas części 1 zadania
	int durationSecondPart; // Czas części 2 zadania
	int timeEndFirstPart; // Moment zakończenia części 1
	int timeEndSecondPart; // Moment zakończenia części 2
};

struct Maintenance {
	int assigment; // Numer maszyny
	int readyTime; // Czas gotowości (pojawienia się)
	int duration; // Czas trwania przerwania
};

// Funkcja pomocnicza używana w sortowaniu przerwań
bool sortMaintenance(Maintenance * i, Maintenance * j) {return (i->readyTime < j->readyTime); }

// Generator przestojóww na maszynie
void GeneratorPrzestojow(vector<Maintenance*> &lista, int liczbaPrzerwanFirstProcessor, int liczbaPrzerwanSecondProcessor, int lowerTimeLimit, int upperTimeLimit, int lowerReadyTime, int upperReadyTime) {
	srand(time(NULL));
	
	int size = (upperReadyTime - lowerReadyTime) + (upperTimeLimit - lowerTimeLimit);
	bool * maintenanceTimeTable = new bool[size]; // Jedna tablica bo przerwania na maszynach nie mogą się nakładać na siebie 
	
	for(int i = 0; i < size; i++) {
		maintenanceTimeTable[i] = false;
	}
	
	int liczbaPrzerwan = liczbaPrzerwanFirstProcessor + liczbaPrzerwanSecondProcessor;
	
	for(int i = 0; i < liczbaPrzerwan; i++) {
		Maintenance * przerwa = new Maintenance;
		
		// Losowanie przerwy na którą maszynę ma trafić
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
			
		// Random punkt startu + sprawdzenie czy jest to możliwe
			int readyTime = 0;
			int startTimeCheck, stopTimeCheck = 0;
			
			while(true) {
				readyTime = lowerReadyTime + (int)(rand() / (RAND_MAX + 1.0) * upperReadyTime);
				
				startTimeCheck = readyTime - lowerReadyTime;
				stopTimeCheck = startTimeCheck + duration;
				// Sprawdzenie czy można dać przerwanie od readyTime
					bool repeatCheck = false;
					for(int j = startTimeCheck; j < stopTimeCheck; j++) {
						if(maintenanceTimeTable[j]) {
							repeatCheck = true;
							break; // Konieczne jest ponowne losowanie czasu rozpoczęcia
						}
					}
					
					if(!repeatCheck) {
						break; // Można opuścić pętle while - znaleziono konfigurację dla przerwania
					}
			}
			
			// Zapis przerwania w tablicy pomocniczej
				for(int j = startTimeCheck; j < stopTimeCheck; j++) {
					maintenanceTimeTable[j] = true;
				}
			
			// Uzupełnienie danych o przerwaniu
				przerwa->readyTime = readyTime;
				przerwa->duration = duration;
			
			// Dodanie przestoju do listy
				lista.push_back(przerwa);
	}
	
	// Czyszczenie pamięci - zwalnianie niepotrzebnych zasobów
		delete maintenanceTimeTable;
}

// Generator instancji problemu
void GeneratorInstancji(vector<Task*> &lista, int n, int lowerTimeLimit, int upperTimeLimit) {
	srand(time(NULL));
	
	for(int i = 0; i < n; i++) {
		Task * zadanie = new Task;
		
		// Przydział maszyny do części 1 zadania
			zadanie->assigment = rand() % 2;
		
		// Randomy na czas trwania zadania
			zadanie->durationFirstPart = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			zadanie->durationSecondPart = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			
		// Czas zakończenia póki co ustawiony na 0 = zadanie nie było używane
			zadanie->timeEndFirstPart = 0;
			zadanie->timeEndSecondPart = 0;
		
		// Dodanie zadania do listy
			lista.push_back(zadanie);	
	}
}

// Zapis instancji do pliku
void ZapiszInstancjeDoPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, int numerInstancjiProblemu) {
	ofstream file;
	file.open("instancje.txt");	
	
	if(file.is_open()) {
		file << "**** " << numerInstancjiProblemu << " ****" << endl;
		
		// Uzupełnienie pliku o wygenerowane czasy pracy
			int iloscZadan = listaZadan.size();
			file << iloscZadan << endl;
			
			for(int i = 0; i < iloscZadan; i++) {
				file << listaZadan[i]->durationFirstPart << ":" << listaZadan[i]->durationSecondPart << ":"
				<< listaZadan[i]->assigment << ":" << 1 - listaZadan[i]->assigment << ";" << endl;
			}
		
		// Uzupełnienie pliku o czasy przestojów maszyn
			int iloscPrzestojow = listaPrzerwan.size();
			for(int i = 0; i < iloscPrzestojow; i++) {
				file << i << ":" << listaPrzerwan[i]->assigment << ":" << listaPrzerwan[i]->duration << ":" << listaPrzerwan[i]->readyTime << ";" << endl;
			}
			
			file << "**** EOF ****" << endl;
	}	
	
	file.close();
}

// Zapis wyników do pliku tekstowego
void ZapiszWynikiDoPliku(vector<Task*> &listaZadan, int numerInstancjiProblemu) {
	ofstream file;
	file.open("wynik.txt");	
	
	if(file.is_open()) {
		file << "**** WYNIKI DLA " << numerInstancjiProblemu << " ****" << endl;
		
		// Uzupełnienie pliku o rozwiązanie
			int iloscZadan = listaZadan.size();			
			for(int i = 0; i < iloscZadan; i++) {
				file << listaZadan[i]->durationFirstPart << ":" << listaZadan[i]->durationSecondPart << ":"
				<< listaZadan[i]->assigment << ":" << 1 - listaZadan[i]->assigment << ":" 
				<< listaZadan[i]->timeEndFirstPart << ":" << listaZadan[i]->timeEndSecondPart << ";" << endl;
			}
	}
}

// Wczytywanie instancji z pliku do pamięci
void WczytajDaneZPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, int &numerInstancjiProblemu) {
	FILE *file;
	file = fopen("instancje.txt", "r");
	
	if(file != NULL) {
		// Pobranie numeru instancji problemu
			fscanf(file, "**** %d ****", &numerInstancjiProblemu);
			
		// Pobranie liczby zadań
			int liczbaZadan = 0;
			fscanf(file, "%d", &liczbaZadan);
		
		// Zmienne pomocnicze w tworzeniu zadania
			int assigmentFirstPart, assigmentSecondPart, durationFirstPart, durationSecondPart;
		
		// Pobranie wartości zadania z pliku instancji
			for(int i = 0; i < liczbaZadan; i++) {
				// Odczyt wpisu
					fscanf(file, "%d:%d:%d:%d;", &durationFirstPart, &durationSecondPart, &assigmentFirstPart, &assigmentSecondPart);		
				
				// Utworzenie zadania
					Task * zadanie = new Task;
					
				// Ustawienie wartości zadania
					zadanie->assigment = assigmentFirstPart;
					zadanie->durationFirstPart = durationFirstPart;
					zadanie->durationSecondPart = durationSecondPart;
					zadanie->timeEndFirstPart = 0;
					zadanie->timeEndSecondPart = 0;
					
				// Dodanie zadania do wektora zadań
					listaZadan.push_back(zadanie);
			}
			
		// Zestaw zmiennych używanych przy odczycie przerwań na maszynach
			int assigment, duration, readyTime, numer;
			int oldNumber = -1;
			
		// Pobranie wartości dotyczących przerwań
			while(fscanf(file, "%d:%d:%d:%d;", &numer, &assigment, &duration, &readyTime)) {
				// Sprawdzenie czy nie mamy zapętlenia
					if(oldNumber == numer)
						break;
						
				// Utworzenie przerwy
					Maintenance * przerwa = new Maintenance;
					
				// Ustawienie wartości zadania
					przerwa->assigment = assigment;
					przerwa->duration = duration;
					przerwa->readyTime = readyTime;
					
				// Dodanie zadania do wektora zadań
					listaPrzerwan.push_back(przerwa);
					
				// Zmienna pomocnicza do eliminacji zapętleń przy odczycie
					oldNumber = numer;
			}
	}
}

// Odczyt przerwań na maszynach na ekran
void OdczytPrzerwan(vector<Maintenance*> &listaPrzerwan) {
	int size = listaPrzerwan.size();
	for(int i = 0; i < size; i++) {
		cout << "Maszyna = " << listaPrzerwan[i]->assigment << " | Start = " << listaPrzerwan[i]->readyTime << " | Czas trwania = " << listaPrzerwan[i]->duration << endl;
	}
}

// Generator rozwiązań losowych
vector<Task*> GeneratorLosowy(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor, int iloscZadan) {
	// Utworzenie kopii zadań aby móc tworzyć swoje rozwiązanie
		vector<Task*> zadaniaLokalne(listaZadan);
	
	// Zmienne używane w przebiegu pracy GeneratoraLosowego	
		int numerPrzerwaniaFirstProcessor = 0; // Numer aktualnego przerwania na procesorze pierwszym
		int numerPrzerwaniaSecondProcessor = 0; // Numer aktualnego przerwania na procesorze drugim
		int count = 0; // Licznik przeliczonych już zadań
		int numerZadania = 0; // Numer aktualnie rozpatrywanego zadania (losowe)
		int najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime; // Czas momentu ROZPOCZĘCIA przerwania na procesorze pierwszym
		int najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime; // Czas momentu ROZPOCZĘCIA przerwania na procesorze drugim 
		int timeFirstProcessor = 0; // Zmienna czasowa - procesor pierwszy
		int timeSecondProcessor = 0; // Zmienna czasowa - procesor drugi
		int maxCount = 2 * iloscZadan; // Ilość koniecznych edycji w zadaniach (part I + part II w każdym zadaniu)
		int listaPrzerwanFirstProcessorSize = listaPrzerwanFirstProcessor.size(); // Ilość przerwań dla pierwszego procesora - aby nie liczyć za każdym razem tej wartości
		int listaPrzerwanSecondProcessorSize = listaPrzerwanSecondProcessor.size(); // Ilość przerwań dla drugiej maszyny - podonie jak wyżej, unikamy niepotrzebnego, wielokrotnego liczenia tej wartości
		
	// Tworzymy dwie tablice pomocnicze do sprawdzania czy zadanie było już uwzględnione
		bool * firstPart = new bool[iloscZadan]; // Część I zadania - czy była uwzględniona (jeśli tak to true)
		bool * secondPart = new bool[iloscZadan]; // Część II zadania - czy była uwzględniona (jeśli tak to true)
	
	// Licznik odwiedzin w każdym z zadań
		int * licznikOdwiedzonych = new int[iloscZadan]; // Licznik odwiedzeń w danym zadaniu aby unikać pętli
		
	// Pętla startowa zerująca tablice
		for(int i = 0; i < iloscZadan; i++) {
			firstPart[i] = false;
			secondPart[i] = false;
			licznikOdwiedzonych[i] = 0;
		}
	
		while(count < maxCount) {
			// Losujemy numer zadania
				numerZadania = rand() % iloscZadan;
			
			// Sprawdzamy czy nie było już przypadkiem użyte całe - w takim przypadku robimy kolejną iterację pętli i losujemy na nowo numer zadania
				if(firstPart[numerZadania] && secondPart[numerZadania])
					continue; // Skok do kolejnej iteracji
				
				if(DEBUG) {
					debugFile << "Numer = " << numerZadania << " M" << zadaniaLokalne[numerZadania]->assigment 
					<< " czasy: " << timeFirstProcessor << "|" << timeSecondProcessor 
					<< " przerwy: " << najblizszyMaintenanceFirstProcessor << "|" << najblizszyMaintenanceSecondProcessor << endl;
				}

			// Zadanie nie było jeszcze używane
				if(!firstPart[numerZadania]) {
					// Sprawdzamy typ zadania
					if(zadaniaLokalne[numerZadania]->assigment == 0) { // Przydział na pierwszą maszynę
						// Dwa możliwe przypadki - zadanie umieszczamy przed przerwą lub po niej
						
						// Sprawdzamy czy zadanie można umieścić przed maintenance najbliższym (jeżeli jest  on -1 to już nie wystąpi)
						if((timeFirstProcessor + zadaniaLokalne[numerZadania]->durationFirstPart) <= najblizszyMaintenanceFirstProcessor
							|| (najblizszyMaintenanceFirstProcessor == -1)) {
							// Ustawiamy czas na maszynie pierwszej
								timeFirstProcessor += zadaniaLokalne[numerZadania]->durationFirstPart;
							
								if(DEBUG) 
									debugFile << "Czas FM: " << timeFirstProcessor << endl;
							
							// Ustawiamy czas zakończenia zadania
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeFirstProcessor;
								
							// Ustawiamy że zadanie zostało użyte (part I)
								firstPart[numerZadania] = true;	
							
						} else { // Nie umieściliśmy zadania przed przerwą
							while(true) {
								// Przesuwamy się na chwilę po przerwaniu
									timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;
								
								// Ustawiamy czas następnego przerwania
									numerPrzerwaniaFirstProcessor++;
									if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
										najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
									else
										najblizszyMaintenanceFirstProcessor = -1;
										
								// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
									if((timeFirstProcessor + zadaniaLokalne[numerZadania]->durationFirstPart) <= najblizszyMaintenanceFirstProcessor
										|| (najblizszyMaintenanceFirstProcessor == -1))
										break;
							}
							
							// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeFirstProcessor (wystarczy zwiększyć ją o długość zadania)
								timeFirstProcessor += zadaniaLokalne[numerZadania]->durationFirstPart;
							
								if(DEBUG)		
									debugFile << "Czas FM " << timeFirstProcessor << endl;
							
							// Ustawiamy zmienną czasową zakończenia zadania
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeFirstProcessor;
								
							// Zaznaczamy w tablicy pomocniczej że część pierwsza zadania była użyta
								firstPart[numerZadania] = true;									
						}
						
						// Zwiększamy ilość zadań jakie przerobiliśmy
							count++;
							
					} else { // Przydział zadania na maszynę nr 2
						
						// Sprawdzamy czy zadanie można umieścić przed maintenance najbliższym (jeżeli jest  on -1 to już nie wystąpi)
						if((timeSecondProcessor + zadaniaLokalne[numerZadania]->durationFirstPart) <= najblizszyMaintenanceSecondProcessor
							|| (najblizszyMaintenanceSecondProcessor == -1)) {
							// Ustawiamy czas na maszynie drugiej
								timeSecondProcessor += zadaniaLokalne[numerZadania]->durationFirstPart;
							
								if(DEBUG)
									debugFile << "Czas SM: " << timeSecondProcessor << endl;
							
							// Ustawiamy czas zakończenia zadania
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeSecondProcessor;
								
							// Ustawiamy że zadanie zostało użyte (part I)
								firstPart[numerZadania] = true;	
							
						} else { // Nie umieściliśmy zadania przed przerwą
							while(true) {
								// Przesuwamy się na chwilę po przerwaniu
									timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;
								
								// Ustawiamy czas następnego przerwania
									numerPrzerwaniaSecondProcessor++;
									if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
										najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
									else
										najblizszyMaintenanceSecondProcessor = -1;
										
									if(DEBUG)
										debugFile << "Druga = " << timeSecondProcessor << " oraz " << najblizszyMaintenanceSecondProcessor << endl;
										
								// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
									if((timeSecondProcessor + zadaniaLokalne[numerZadania]->durationFirstPart) <= najblizszyMaintenanceSecondProcessor
										|| (najblizszyMaintenanceSecondProcessor == -1))
										break;
							}
							
							// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeSecondProcessor(wystarczy zwiększyć ją o długość zadania)
								timeSecondProcessor += zadaniaLokalne[numerZadania]->durationFirstPart;
							
								if(DEBUG)		
									debugFile << "Czas SM " << timeSecondProcessor << endl;
							
							// Ustawiamy zmienną czasową zakończenia zadania
								zadaniaLokalne[numerZadania]->timeEndFirstPart = timeSecondProcessor;
								
							// Zaznaczamy w tablicy pomocniczej że część pierwsza zadania była użyta
								firstPart[numerZadania] = true;									
						}
						
						// Zwiększamy ilość zadań jakie przerobiliśmy
							count++;
					}
				} else {
					// PRZYDZIELAMY DRUGĄ CZĘŚĆ ZADANIA
					// Mogą wystąpić problemy z zapętleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartość
					licznikOdwiedzonych[numerZadania]++;
					
					// Sprawdzamy typ zadania
					if(zadaniaLokalne[numerZadania]->assigment == 0) { // Przydział na pierwszą maszynę
						// Sprawdzamy czy czas na maszynie nie jest mniejszy od zakończenia się pierwszej części
						if(timeSecondProcessor < zadaniaLokalne[numerZadania]->timeEndFirstPart) {
							// Sprawdzamy czy nie jesteśmy po raz x w pętli
							if(licznikOdwiedzonych[numerZadania] >= MIN_TASK_COUNTER) {
								if(DEBUG)
									debugFile << "Przestawiono czas! M1" << endl;
								// Tworzymy pomocniczą zmienną odległości
									int minTime = INT_MAX;
									int tempTime = 0;
									
								// Resetujemy liczniki i patrzymy na odległości
									for(int i = 0; i < iloscZadan; i++) {
										licznikOdwiedzonych[i] = 0;
										
										if(!secondPart[i]) {
											int tempTime = zadaniaLokalne[i]->timeEndFirstPart - timeSecondProcessor;
											if(tempTime < minTime)
												minTime = tempTime;
										}
									}
									
								// Przestawiamy czas na maszynie
									timeSecondProcessor += minTime;
									
							} else // Jeżeli nie mamy osiągniętej wartości to pomijamy iterację
								continue;
						}
							
						// Zadanie można umieścić
							// Sprawdzamy czy zadanie można umieścić przed maintenance najbliższym (jeżeli jest  on -1 to już nie wystąpi)
							if((timeSecondProcessor + zadaniaLokalne[numerZadania]->durationSecondPart) <= najblizszyMaintenanceSecondProcessor
								|| (najblizszyMaintenanceSecondProcessor == -1)) {
								// Ustawiamy czas na maszynie pierwszej
									timeSecondProcessor += zadaniaLokalne[numerZadania]->durationSecondPart;
								
								// Ustawiamy czas zakończenia zadania
									zadaniaLokalne[numerZadania]->timeEndSecondPart = timeSecondProcessor;
									
								// Ustawiamy że zadanie zostało użyte (part II)
									secondPart[numerZadania] = true;	
							
							} else { // Nie umieściliśmy zadania przed przerwą
								while(true) {
									// Przesuwamy się na chwilę po przerwaniu
										timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;
									
									// Ustawiamy czas następnego przerwania
										numerPrzerwaniaSecondProcessor++;
										if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
											najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
										else
											najblizszyMaintenanceSecondProcessor = -1;
											
									// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
										if((timeSecondProcessor + zadaniaLokalne[numerZadania]->durationSecondPart) <= najblizszyMaintenanceSecondProcessor
											|| (najblizszyMaintenanceSecondProcessor == -1))
											break;
								}
								
								// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeSecondProcessor (wystarczy zwiększyć ją o długość zadania)
									timeSecondProcessor += zadaniaLokalne[numerZadania]->durationSecondPart;
										
								// Ustawiamy zmienną czasową zakończenia zadania
									zadaniaLokalne[numerZadania]->timeEndSecondPart = timeSecondProcessor;
									
								// Zaznaczamy w tablicy pomocniczej że część pierwsza zadania była użyta
									secondPart[numerZadania] = true;
							}
						
							// Zwiększamy ilość zadań jakie przerobiliśmy
								count++;
					} else {
						// Sprawdzamy czy czas na maszynie nie jest mniejszy od zakończenia się pierwszej części
						if(timeFirstProcessor < zadaniaLokalne[numerZadania]->timeEndFirstPart) {
							// Sprawdzamy czy nie jesteśmy po raz x w pętli
							if(licznikOdwiedzonych[numerZadania] >= MIN_TASK_COUNTER) {
								if(DEBUG)
									debugFile << "Przestawiono czas! M0" << endl;
									
								// Tworzymy pomocniczą zmienną odległości
									int minTime = INT_MAX;
									int tempTime = 0;
									
								// Resetujemy liczniki i patrzymy na odległości
									for(int i = 0; i < iloscZadan; i++) {
										licznikOdwiedzonych[i] = 0;
										
										if(!secondPart[i]) {
											tempTime = zadaniaLokalne[i]->timeEndFirstPart - timeFirstProcessor;
											if(tempTime < minTime)
												minTime = tempTime;
										}
									}
									
								// Przestawiamy czas na maszynie
									timeFirstProcessor += minTime;
									
							} else // Jeżeli nie mamy osiągniętej wartości to pomijamy iterację
								continue;
						}
							
						// Zadanie można umieścić
							// Sprawdzamy czy zadanie można umieścić przed maintenance najbliższym (jeżeli jest  on -1 to już nie wystąpi)
							if((timeFirstProcessor + zadaniaLokalne[numerZadania]->durationSecondPart) <= najblizszyMaintenanceFirstProcessor
								|| (najblizszyMaintenanceFirstProcessor == -1)) {
								// Ustawiamy czas na maszynie pierwszej
									timeFirstProcessor += zadaniaLokalne[numerZadania]->durationSecondPart;
								
								// Ustawiamy czas zakończenia zadania
									zadaniaLokalne[numerZadania]->timeEndSecondPart = timeFirstProcessor;
									
								// Ustawiamy że zadanie zostało użyte (part II)
									secondPart[numerZadania] = true;	
							
							} else { // Nie umieściliśmy zadania przed przerwą
								while(true) {
									// Przesuwamy się na chwilę po przerwaniu
										timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;
									
									// Ustawiamy czas następnego przerwania
										numerPrzerwaniaFirstProcessor++;
										if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
											najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
										else
											najblizszyMaintenanceFirstProcessor = -1;
											
									// Musismy sprawdzić czy uda się nam wcisnąć nasze zadanie
										if((timeFirstProcessor + zadaniaLokalne[numerZadania]->durationSecondPart) <= najblizszyMaintenanceFirstProcessor
											|| (najblizszyMaintenanceFirstProcessor == -1))
											break;
								}
								
								// Po opuszczeniu pętli mamy poprawną wartość w zmiennej timeSecondProcessor (wystarczy zwiększyć ją o długość zadania)
									timeFirstProcessor += zadaniaLokalne[numerZadania]->durationSecondPart;
										
								// Ustawiamy zmienną czasową zakończenia zadania
									zadaniaLokalne[numerZadania]->timeEndSecondPart = timeFirstProcessor;
									
								// Zaznaczamy w tablicy pomocniczej że część pierwsza zadania była użyta
									secondPart[numerZadania] = true;
							}
						
							// Zwiększamy ilość zadań jakie przerobiliśmy
								count++;
					}
				}
		}
		
		// Czyszczenie pamięci - zwalnianie niepotrzebnych zasobów
			delete firstPart;
			delete secondPart;
			delete licznikOdwiedzonych;
		
		return zadaniaLokalne;
}

// Podział przerwań na dwie listy - każda maszyna osobno
void PodzielPrzerwyNaMaszyny(vector<Maintenance*> &listaPrzerwan, vector<Maintenance*> &przerwaniaFirstProcessor, vector<Maintenance*> &przerwaniaSecondProcessor) {
	// Zmienna pomocnicza by skrócić czas pracy (nie trzeba x razy liczyć)
		int size = listaPrzerwan.size();

	//Sprawdzamy do jakiej maszyny przypisane mamy przerwanie	
		for(int i = 0; i < size; i++) {
			Maintenance * przerwa = listaPrzerwan[i];
			if(przerwa->assigment == 0) {
				przerwaniaFirstProcessor.push_back(przerwa);
			} else {
				przerwaniaSecondProcessor.push_back(przerwa);
			}
		}	
}

// Odczyt danych zadań na ekran
void OdczytDanychZadan(vector<Task*> &listaZadan) {
	int size = listaZadan.size();
	for(int i = 0; i < size; i++) {
		cout << "--- ID: " << i << " przydzial: M" << listaZadan[i]->assigment << " duration = " << listaZadan[i]->durationFirstPart << "|" << listaZadan[i]->durationSecondPart 
		<< " --- zakonczenie = " << listaZadan[i]->timeEndFirstPart << "|" << listaZadan[i]->timeEndSecondPart << " --- " << endl;
	}
}

// Sortowanie przerwań według rosnącego czasu rozpoczęcia
void SortujPrzerwania(vector<Maintenance*> &listaPrzerwan) {
	// Używamy algorytmicznej funkcji sort z ustawionym trybem sortowania aby przyspieszyć pracę
		sort(listaPrzerwan.begin(), listaPrzerwan.end(), sortMaintenance);
}

// Tworzenie timeline dla obserwacji wyników pracy
void UtworzGraf(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, long int wynik) {
	int iloscZadan = listaZadan.size(); // Ilość zadań w systemie
	int iloscPrzerwan = listaPrzerwan.size(); // Ilość okresów przestojów na maszynach
	
	ofstream file;
	file.open("index.html");
	file << "<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" /><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
	file << "<title>OK - Wyniki pracy generatora</title></head><body><script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>";
	file << "<script type=\"text/javascript\">google.charts.load(\"current\", {packages:[\"timeline\"]});google.charts.setOnLoadCallback(drawChart);function drawChart() {";
	file << "var container = document.getElementById('example4.2');var chart = new google.visualization.Timeline(container);var dataTable = new google.visualization.DataTable();";
	file << "dataTable.addColumn({ type: 'string', id: 'Role' });dataTable.addColumn({ type: 'string', id: 'Name' });dataTable.addColumn({ type: 'number', id: 'Start' });dataTable.addColumn({ type: 'number', id: 'End' });dataTable.addRows([";
	
	int timeStart = 0;
	int timeStop = 0;

	// Zapisujemy do pliku nasze zadania
		for(int i = 0; i < iloscZadan; i++) {
			timeStop = listaZadan[i]->timeEndFirstPart;
			timeStart = timeStop - listaZadan[i]->durationFirstPart;

			file << "[ 'M" << listaZadan[i]->assigment + 1 << "', 'Zadanie " << i + 1 << "', " << timeStart << "," << timeStop << "]," << endl;
			
			timeStop = listaZadan[i]->timeEndSecondPart;
			timeStart = timeStop - listaZadan[i]->durationSecondPart;

			file << "[ 'M" << 2 - listaZadan[i]->assigment << "', 'Zadanie " << i + 1 << "', " << timeStart << ", " << timeStop << " ]," << endl;
		}
		
	// Zapis przerwań
		for(int i = 0; i < iloscPrzerwan; i++) {
			timeStart = listaPrzerwan[i]->readyTime;
			timeStop = timeStart + listaPrzerwan[i]->duration;
			
			if(i + 1 == iloscPrzerwan) { // Ostatnia iteracja
				file << "[ 'M" << listaPrzerwan[i]->assigment + 1 << "', 'PRZERWANIE " << i + 1 << "', " << timeStart << ", " << timeStop << " ]]);" << endl;
				file << "var options = {timeline: { groupByRowLabel: true }};chart.draw(dataTable, options);}</script><div id=\"example4.2\" style=\"height: 200px;\"></div><br><div><span>Wartosc funkcji celu: " << wynik << "</span></div></body></html>" << endl;
			} else {
				file << "[ 'M" << listaPrzerwan[i]->assigment + 1 << "', 'PRZERWANIE " << i + 1 << "', " << timeStart << ", " << timeStop << " ]," << endl;
			}
		}
}

// Obliczanie wartości funkcji celu
long int ObliczFunkcjeCelu(vector<Task*> &listaZadan) {
	int size = listaZadan.size();
	long int sum = 0;
	
	for(int i = 0; i < size; i++) {
		sum += listaZadan[i]->timeEndFirstPart + listaZadan[i]->timeEndSecondPart;
	}
	
	return sum;
}

int main() {
	debugFile.open("debug.txt");
	int rozmiarInstancji = INSTANCE_SIZE;
	int numerInstancjiProblemu = INSTANCE_NUMBER;

	// Utworzenie wektora na n zadań
		vector<Task*> listaZadan;

	// Wektor przerwań pracy na maszynach
		vector<Maintenance*> listaPrzerwan; 

	// Wygenerowanie zadań
		 GeneratorInstancji(listaZadan, rozmiarInstancji, LOWER_TIME_TASK_LIMIT, UPPER_TIME_TASK_LIMIT);

//	 Wygenerowanie przerwań	
		 GeneratorPrzestojow(listaPrzerwan, MAINTENANCE_FIRST_PROCESSOR, MAINTENANCE_SECOND_PROCESSOR, LOWER_TIME_MAINTENANCE_LIMIT, UPPER_TIME_MAINTENANCE_LIMIT, LOWER_READY_TIME_MAINTENANCE_LIMIT, UPPER_READY_TIME_MAINTENANCE_LIMIT);
		 OdczytPrzerwan(listaPrzerwan);

	// Zapis danych do pliku
		ZapiszInstancjeDoPliku(listaZadan, listaPrzerwan, numerInstancjiProblemu);

	// Wczytanie danych z pliku
//		WczytajDaneZPliku(listaZadan, listaPrzerwan, numerInstancjiProblemu);
//		OdczytPrzerwan(listaPrzerwan);
		
		vector<Maintenance*> przerwaniaFirstProcessor;
		vector<Maintenance*> przerwaniaSecondProcessor;
		SortujPrzerwania(listaPrzerwan);
		PodzielPrzerwyNaMaszyny(listaPrzerwan, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
		
		GeneratorLosowy(listaZadan, przerwaniaFirstProcessor, przerwaniaSecondProcessor, rozmiarInstancji);
		
//		OdczytDanychZadan(listaZadan);
		
		ZapiszWynikiDoPliku(listaZadan, numerInstancjiProblemu);
		
		long int wynik = ObliczFunkcjeCelu(listaZadan);
		UtworzGraf(listaZadan, listaPrzerwan, wynik);		
	
	// Czyszczenie pamięci - zwalnianie niepotrzebnych zasobów
		przerwaniaFirstProcessor.clear();
		przerwaniaSecondProcessor.clear();
		listaPrzerwan.clear();
		listaZadan.clear();
	
	return 0;
}
