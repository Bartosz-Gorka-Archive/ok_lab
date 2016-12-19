/* OPTYMALIZACJA KOMBINATORYCZNA
 * Temat: Metaheurystyki w problemie szeregowania zadaĹ„
 * Autor: Bartosz GĂłrka, Mateusz Kruszyna
 * Data: GrudzieĹ„ 2016r.
*/

// Biblioteki uĹĽywane w programie
#include <iostream> // I/O stream
#include <vector> // Vector table
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <climits> // INT_MAX do generatora losowego
#include <algorithm> // Sortowanie przerwaĹ„

using namespace std;

// DEFINICJE GLOBALNE
#define DEBUG true // TRYB DEBUGOWANIA [true / false]
#define MIN_TASK_COUNTER 30 // PO ilu iteracjach sprawdzaÄ‡ zapÄ™tlenie [WartoĹ›Ä‡ liczbowa > 0]

#define LOWER_TIME_TASK_LIMIT 5 // Dolne ograniczenie dĹ‚ugoĹ›ci zadania [WartoĹ›Ä‡ liczbowa > 0]
#define UPPER_TIME_TASK_LIMIT 15 // GĂłrne ograniczenie dĹ‚ugoĹ›ci zadania [WartoĹ›Ä‡ liczbowa > 0]

#define MAINTENANCE_FIRST_PROCESSOR 3 // IloĹ›Ä‡ przerwaĹ„ na pierwszej maszynie [WartoĹ›Ä‡ liczbowa >= 0]
#define MAINTENANCE_SECOND_PROCESSOR 3 // IloĹ›Ä‡ przerwaĹ„ na drugiej maszynie [WartoĹ›Ä‡ liczbowa >= 0]

#define LOWER_TIME_MAINTENANCE_LIMIT 5 // Dolne ograniczenie dĹ‚ugoĹ›ci przerwania [WartoĹ›Ä‡ liczbowa >= 0]
#define UPPER_TIME_MAINTENANCE_LIMIT 20 // GĂłrne ograniczenie dĹ‚ugoĹ›ci przerwania [WartoĹ›Ä‡ liczbowa > 0]

#define LOWER_READY_TIME_MAINTENANCE_LIMIT 0 // Dolne ograniczenie czasu gotowoĹ›ci przerwania [WartoĹ›Ä‡ liczbowa >= 0]
#define UPPER_READY_TIME_MAINTENANCE_LIMIT 200 // GĂłrne ograniczenie czasu gotowoĹ›ci przerwania [WartoĹ›Ä‡ liczbowa > 0]

#define INSTANCE_SIZE 5 // Rozmiar instancji problemu
#define INSTANCE_NUMBER 1 // Numer instancji problemu (moĹĽe byÄ‡ zmieniana przy odczycie danych z pliku)
#define MAX_SOLUTIONS 3 // IloĹ›Ä‡ rozwiÄ…zaĹ„ jakie chcemy wygenerowaÄ‡
#define MAX_SOLUTION_AFTER_MUTATION 9 // IloĹ›Ä‡ rozwiÄ…zaĹ„ po mutacji (ile ta mutacja ma stworzyc rozwiazan w sumie)

#define MAX_DURATION_PROGRAM_TIME 0.3 // Maksymalna dĹ‚ugoĹ›Ä‡ trwania programu w SEKUNDACH
#define PROBABILTY_OF_RANDOM_GENERATION 30 // prawdopodobieĹ„stwo stworzenia rozwiÄ…zaĹ„ przez los (dopeĹ‚nienie to przez macierz fermonowÄ…

#define PROCENT_ZANIKANIA 5 // ile procent sladu fermonowego ma zanikac co iteracje
#define WYKLADNIK_POTEGI 3.5 // potrzebny do funkcji splaszczajacej
#define CO_ILE_ITERACJI_WIERSZ 2 // co ile itarcji przeskakujemy do kolejnego wiersza >=1 (gdy 1 to cyklicznie przechodzimy, czesciej sie nie da;p)

ofstream debugFile; // Zmienna globalna uĹĽywana przy DEBUG mode
long int firstSolutionValue; // Zmienna globalna najlepszego rozwiÄ…zania wygenerowanego przez generator losowy

double macierzFermonowa[INSTANCE_SIZE][INSTANCE_SIZE];
// Struktura danych w pamiÄ™ci
struct Task{
	int ID; // ID zadania
	int part; // Numer czÄ™Ĺ›ci zadania [0, 1]
	int assigment; // PrzydziaĹ‚ zadania do maszyny [0, 1]
	int duration; // DĹ‚ugoĹ›Ä‡ zadania
	int endTime; // Moment zakoĹ„czenia
	Task *anotherPart; // WskaĹşnik na komplementarne zadanie
};

struct Maintenance {
	int assigment; // Numer maszyny
	int readyTime; // Czas gotowoĹ›ci (pojawienia siÄ™)
	int duration; // Czas trwania przerwania
};

// Funkcja pomocnicza uĹĽywana w sortowaniu przerwaĹ„
bool sortMaintenance(Maintenance * i, Maintenance * j) {return (i->readyTime < j->readyTime); }

// Pomocnicze funkcje uĹĽywane przy sortowaniu zadaĹ„
bool sortTask(Task *i, Task *j) { return (i->endTime < j->endTime); }
bool sortTaskByID(Task *i, Task *j) { return (i->ID < j->ID); } // Po wartoĹ›ci ID

void KopiujDaneOperacji(vector<Task*> &listaWejsciowa, vector<Task*> &listaWyjsciowa);

// Generator przestojĂłww na maszynie
void GeneratorPrzestojow(vector<Maintenance*> &lista, int liczbaPrzerwanFirstProcessor, int liczbaPrzerwanSecondProcessor, int lowerTimeLimit, int upperTimeLimit, int lowerReadyTime, int upperReadyTime) {
	int size = (upperReadyTime - lowerReadyTime) + (upperTimeLimit - lowerTimeLimit);
	bool * maintenanceTimeTable = new bool[size]; // Jedna tablica bo przerwania na maszynach nie mogÄ… siÄ™ nakĹ‚adaÄ‡ na siebie

	for(int i = 0; i < size; i++) {
		maintenanceTimeTable[i] = false;
	}

	int liczbaPrzerwan = liczbaPrzerwanFirstProcessor + liczbaPrzerwanSecondProcessor;

	for(int i = 0; i < liczbaPrzerwan; i++) {
		Maintenance * przerwa = new Maintenance;

		// Losowanie przerwy na ktĂłrÄ… maszynÄ™ ma trafiÄ‡
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

		// Random punkt startu + sprawdzenie czy jest to moĹĽliwe
			int readyTime = 0;
			int startTimeCheck, stopTimeCheck = 0;

			while(true) {
				readyTime = lowerReadyTime + (int)(rand() / (RAND_MAX + 1.0) * upperReadyTime);

				startTimeCheck = readyTime - lowerReadyTime;
				stopTimeCheck = startTimeCheck + duration;
				// Sprawdzenie czy moĹĽna daÄ‡ przerwanie od readyTime
					bool repeatCheck = false;
					for(int j = startTimeCheck; j < stopTimeCheck; j++) {
						if(maintenanceTimeTable[j]) {
							repeatCheck = true;
							break; // Konieczne jest ponowne losowanie czasu rozpoczÄ™cia
						}
					}

					if(!repeatCheck) {
						break; // MoĹĽna opuĹ›ciÄ‡ pÄ™tle while - znaleziono konfiguracjÄ™ dla przerwania
					}
			}

			// Zapis przerwania w tablicy pomocniczej
				for(int j = startTimeCheck; j < stopTimeCheck; j++) {
					maintenanceTimeTable[j] = true;
				}

			// UzupeĹ‚nienie danych o przerwaniu
				przerwa->readyTime = readyTime;
				przerwa->duration = duration;

			// Dodanie przestoju do listy
				lista.push_back(przerwa);
	}

	// Czyszczenie pamiÄ™ci - zwalnianie niepotrzebnych zasobĂłw
		delete maintenanceTimeTable;
}

// Sortowanie przerwaĹ„ wedĹ‚ug rosnÄ…cego czasu rozpoczÄ™cia
void SortujPrzerwania(vector<Maintenance*> &listaPrzerwan) {
	// UĹĽywamy algorytmicznej funkcji sort z ustawionym trybem sortowania aby przyspieszyÄ‡ pracÄ™
		sort(listaPrzerwan.begin(), listaPrzerwan.end(), sortMaintenance);
}

// Sortowanie zadaĹ„ wedĹ‚ug wzrastajÄ…cego ID
void SortujZadaniaPoID(vector<Task*> &listaZadan) {
	sort(listaZadan.begin(), listaZadan.end(), sortTaskByID);
}

// Sortowanie zadaĹ„ wedĹ‚ug rosnÄ…cego czasu zakoĹ„czenia pracy
void SortujZadaniaPoEndTime(vector<Task*> &listaZadan) {
	sort(listaZadan.begin(), listaZadan.end(), sortTask);
}

void SortujListeZadanPoEndTime(vector< vector<Task*> > &listaRozwiazan){
    int sizeListaRozwiazan = listaRozwiazan.size();
    for(int i=0;i<sizeListaRozwiazan;i++){
        SortujZadaniaPoEndTime(listaRozwiazan[i]);
    }
}
// Generator instancji problemu
void GeneratorInstancji(vector<Task*> &lista, int maxTask, int lowerTimeLimit, int upperTimeLimit) {
	int assigment = 0;

	for(int i = 0; i < maxTask; i++) {
		Task * taskFirst = new Task;
		Task * taskSecond = new Task;

		// PowiÄ…zujemy miÄ™dzy sobÄ… zadania
			taskFirst->anotherPart = taskSecond;
			taskSecond->anotherPart = taskFirst;

		// Przypisujemy pararametr part
			taskFirst->part = 0;
			taskSecond->part = 1;

		// Przypisujemy ID zadania
			taskFirst->ID = i + 1;
			taskSecond->ID = i + 1;

		// Losujemy przydziaĹ‚ czÄ™Ĺ›ci pierwszej zadania na maszynÄ™
			assigment = rand() % 2;

		// UzupeĹ‚niamy dane przydziaĹ‚Ăłw
			taskFirst->assigment = assigment;
			taskSecond->assigment = 1 - assigment;

		// Randomy na czas trwania zadaĹ„
			taskFirst->duration = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);
			taskSecond->duration = lowerTimeLimit + (int)(rand() / (RAND_MAX + 1.0) * upperTimeLimit);

		// Czas zakoĹ„czenia pĂłki co ustawiony na 0 = zadania nie byĹ‚y jeszcze zakolejkowane
			taskFirst->endTime = 0;
			taskSecond->endTime = 0;

		// Dodanie zadaĹ„ do listy
			lista.push_back(taskFirst);
			lista.push_back(taskSecond);
	}
}

// Zapis instancji do pliku
void ZapiszInstancjeDoPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, int numerInstancjiProblemu, string nameParam) {
	// Zmienna pliku docelowego
		ofstream file;

	// Utworzenie zmiennej pomocniczej w postaci nazwy pliku aby mĂłc parametryzowaÄ‡ zapis danych
		string fileName = "instancje_" + nameParam + ".txt";
		file.open(fileName.c_str());

	if(file.is_open()) {
		file << "**** " << numerInstancjiProblemu << " ****" << endl;

		// Obliczenie iloĹ›ci zadaĹ„ w otrzymanym wektorze
			int iloscZadan = listaZadan.size();

		// Posortowanie wektora po wartoĹ›ci ID aby mieÄ‡ obok siebie operacje z tego samego zadania
			SortujZadaniaPoID(listaZadan);

		// Przypisanie do pliku iloĹ›ci zadaĹ„ w instancji
			file << iloscZadan / 2 << endl;

		// UzupeĹ‚nienie pliku o wygenerowane czasy pracy
			for(int i = 0; i < iloscZadan; i += 2) {
				// Dodanie linii z opisem zadania do pliku instancji
					if(listaZadan[i]->part == 0) { // Pod i mamy zadanie bÄ™dÄ…ce Part I
						file << listaZadan[i]->duration << ":" << listaZadan[i]->anotherPart->duration << ":" << listaZadan[i]->assigment << ":" << listaZadan[i]->anotherPart->assigment << ";" << endl;
					} else {
						file << listaZadan[i]->anotherPart->duration << ":" << listaZadan[i]->duration << ":" << listaZadan[i]->anotherPart->assigment << ":" << listaZadan[i]->assigment << ";" << endl;
					}
			}

		// UzupeĹ‚nienie pliku o czasy przestojĂłw maszyn
			int iloscPrzestojow = listaPrzerwan.size();
			for(int i = 0; i < iloscPrzestojow; i++) {
				file << i << ":" << listaPrzerwan[i]->assigment << ":" << listaPrzerwan[i]->duration << ":" << listaPrzerwan[i]->readyTime << ";" << endl;
			}

			file << "**** EOF ****" << endl;
	}

	file.close();
}

// Wczytywanie instancji z pliku do pamiÄ™ci
void WczytajDaneZPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, int &numerInstancjiProblemu, string nameParam) {
	FILE *file;
	string fileName = "instancje_" + nameParam + ".txt";
	file = fopen(fileName.c_str(), "r");

	if(file != NULL) {
		// Pobranie numeru instancji problemu
			fscanf(file, "**** %d ****", &numerInstancjiProblemu);

		// Pobranie liczby zadaĹ„
			int liczbaZadan = 0;
			fscanf(file, "%d", &liczbaZadan);

		// Zmienne pomocnicze w tworzeniu zadania
			int assigmentFirstPart, assigmentSecondPart, durationFirstPart, durationSecondPart;

		// Pobranie wartoĹ›ci zadania z pliku instancji
			for(int i = 0; i < liczbaZadan; i++) {
				// Odczyt wpisu
					fscanf(file, "%d:%d:%d:%d;", &durationFirstPart, &durationSecondPart, &assigmentFirstPart, &assigmentSecondPart);

				// Utworzenie zadania
					Task * taskFirst = new Task;
					Task * taskSecond = new Task;

				// PowiÄ…zanie zadaĹ„
					taskFirst->anotherPart = taskSecond;
					taskSecond->anotherPart = taskFirst;

				// Ustawienie wartoĹ›ci zadaĹ„
					taskFirst->ID = i + 1;
					taskFirst->assigment = assigmentFirstPart;
					taskFirst->duration = durationFirstPart;
					taskFirst->endTime = 0;
					taskFirst->part = 0;

					taskSecond->ID = i + 1;
					taskSecond->assigment = assigmentSecondPart;
					taskSecond->duration = durationSecondPart;
					taskSecond->endTime = 0;
					taskSecond->part = 1;

				// Dodanie zadania do wektora zadaĹ„
					listaZadan.push_back(taskFirst);
					listaZadan.push_back(taskSecond);
			}

		// Zestaw zmiennych uĹĽywanych przy odczycie przerwaĹ„ na maszynach
			int assigment, duration, readyTime, numer;
			int oldNumber = -1;

		// Pobranie wartoĹ›ci dotyczÄ…cych przerwaĹ„
			while(fscanf(file, "%d:%d:%d:%d;", &numer, &assigment, &duration, &readyTime)) {
				// Sprawdzenie czy nie mamy zapÄ™tlenia
					if(oldNumber == numer)
						break;

				// Utworzenie przerwy
					Maintenance * przerwa = new Maintenance;

				// Ustawienie wartoĹ›ci zadania
					przerwa->assigment = assigment;
					przerwa->duration = duration;
					przerwa->readyTime = readyTime;

				// Dodanie zadania do wektora zadaĹ„
					listaPrzerwan.push_back(przerwa);

				// Zmienna pomocnicza do eliminacji zapÄ™tleĹ„ przy odczycie
					oldNumber = numer;
			}
	}
}

// Odczyt przerwaĹ„ na maszynach na ekran
void OdczytPrzerwan(vector<Maintenance*> &listaPrzerwan) {
	int size = listaPrzerwan.size();
	for(int i = 0; i < size; i++) {
		cout << "Maszyna = " << listaPrzerwan[i]->assigment << " | Start = " << listaPrzerwan[i]->readyTime << " | Czas trwania = " << listaPrzerwan[i]->duration << endl;
	}
}

// Generator rozwiÄ…zaĹ„ losowych
vector<Task*> GeneratorLosowy(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor) {
	// Utworzenie kopii zadaĹ„ aby mĂłc tworzyÄ‡ swoje rozwiÄ…zanie
		vector<Task*> zadaniaLokalne;
		KopiujDaneOperacji(listaZadan, zadaniaLokalne);

	// Zmienne uĹĽywane w przebiegu pracy Generatora Losowego
		int iloscZadan = listaZadan.size() / 2;	// IloĹ›Ä‡ zadaĹ„ (iloĹ›Ä‡ operacji / 2)
		Task * currentTask = NULL; // Zmmienna operacyjna aby uproĹ›ciÄ‡ zapis
		int numerPrzerwaniaFirstProcessor = 0; // Numer aktualnego przerwania na procesorze pierwszym
		int numerPrzerwaniaSecondProcessor = 0; // Numer aktualnego przerwania na procesorze drugim
		int count = 0; // Licznik przeliczonych juĹĽ zadaĹ„
		int najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime; // Czas momentu ROZPOCZÄCIA przerwania na procesorze pierwszym
		int najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime; // Czas momentu ROZPOCZÄCIA przerwania na procesorze drugim
		int timeFirstProcessor = 0; // Zmienna czasowa - procesor pierwszy
		int timeSecondProcessor = 0; // Zmienna czasowa - procesor drugi
		int maxCount = 2 * iloscZadan; // IloĹ›Ä‡ koniecznych edycji w zadaniach (part I + part II w kaĹĽdym zadaniu)
		int listaPrzerwanFirstProcessorSize = listaPrzerwanFirstProcessor.size(); // IloĹ›Ä‡ przerwaĹ„ dla pierwszego procesora - aby nie liczyÄ‡ za kaĹĽdym razem tej wartoĹ›ci
		int listaPrzerwanSecondProcessorSize = listaPrzerwanSecondProcessor.size(); // IloĹ›Ä‡ przerwaĹ„ dla drugiej maszyny - podobnie jak wyĹĽej, unikamy niepotrzebnego, wielokrotnego liczenia tej wartoĹ›ci
		int taskID = 0; // Numer zadania
		int pozycja = 0; // Numer aktualnie rozpatrywanego zadania (losowa wartoĹ›Ä‡ z z przedziaĹ‚u 0 - ilosc zadan*2)

	// Tworzymy dwie tablice pomocnicze do sprawdzania czy zadanie byĹ‚o juĹĽ uwzglÄ™dnione
		bool * firstPart = new bool[iloscZadan]; // CzÄ™Ĺ›Ä‡ I zadania - czy byĹ‚a uwzglÄ™dniona (jeĹ›li tak to true)
		bool * secondPart = new bool[iloscZadan]; // CzÄ™Ĺ›Ä‡ II zadania - czy byĹ‚a uwzglÄ™dniona (jeĹ›li tak to true)

	// Licznik odwiedzin w kaĹĽdym z zadaĹ„
		int * licznikOdwiedzonych = new int[iloscZadan]; // Licznik odwiedzeĹ„ w danym zadaniu aby unikaÄ‡ pÄ™tli

	// PÄ™tla startowa zerujÄ…ca tablice
		for(int i = 0; i < iloscZadan; i++) {
			firstPart[i] = false;
			secondPart[i] = false;
			licznikOdwiedzonych[i] = 0;
		}

		while(count < maxCount) {
			// Losujemy pozycjÄ™ w tablicy zadaĹ„
				pozycja = rand() % maxCount;

			// Sprawdzamy zadanie odpowiadajÄ…ce wylosowanej pozycji nie byĹ‚o juĹĽ przypadkiem uĹĽyte caĹ‚e - w takim przypadku losujemy na nowo numer pozycji w tablicy
				taskID = zadaniaLokalne[pozycja]->ID - 1;
				if(firstPart[taskID] && secondPart[taskID])
					continue; // Skok do kolejnej iteracji

				if(DEBUG) {
					debugFile << "Wylosowano = " << pozycja << " Zadanie nr " << taskID << " (Part " << zadaniaLokalne[pozycja]->part + 1 << ")"
					<< " Parametry zadania = " << zadaniaLokalne[pozycja]->assigment << "|" << zadaniaLokalne[pozycja]->duration << "|" << zadaniaLokalne[pozycja]->endTime
					<< " Parametry komplementarnej czÄ™Ĺ›ci = " << zadaniaLokalne[pozycja]->anotherPart->assigment << "|" << zadaniaLokalne[pozycja]->anotherPart->duration << "|" << zadaniaLokalne[pozycja]->anotherPart->endTime
					<< " czasy: " << timeFirstProcessor << "|" << timeSecondProcessor << " przerwy: " << najblizszyMaintenanceFirstProcessor << "|" << najblizszyMaintenanceSecondProcessor << endl;
				}

			// Zadanie nie byĹ‚o jeszcze uĹĽywane
				if(!firstPart[taskID]) {
					// Sprawdzamy typ zadania - jeĹĽeli jest zero to podstawiamy pod zmiennÄ… pomocniczÄ…
						if(zadaniaLokalne[pozycja]->part == 0) {
							currentTask = zadaniaLokalne[pozycja];
						} else { // JeĹĽeli nie - konieczne jest podstawienie czÄ™Ĺ›ci komplementarnej wylosowanego zadania
							currentTask = zadaniaLokalne[pozycja]->anotherPart;
						}

					// Sprawdzamy czy zadanie powinno trafiÄ‡ na maszynÄ™ 0
						if(currentTask->assigment == 0) {
							// Sprawdzamy czy zadanie uda siÄ™ ustawiÄ‡ przed najblizszym maintenance na maszynie
								if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
									// Ustawiamy czas na maszynie pierwszej
										timeFirstProcessor += currentTask->duration;

									if(DEBUG)
										debugFile << "Czas FM: " << timeFirstProcessor << endl;

									// Ustawiamy czas zakoĹ„czenia Part I
										currentTask->endTime = timeFirstProcessor;

									// Ustawiamy ĹĽe zadanie zostaĹ‚o uĹĽyte (Part I)
										firstPart[taskID] = true;

								} else { // Nie udaĹ‚o siÄ™ umieĹ›ciÄ‡ zadania przed przerwÄ…
									while(true) {
										// Przesuwamy siÄ™ na chwilÄ™ po przerwaniu
											timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;

										// Ustawiamy czas nastÄ™pnego przerwania
											numerPrzerwaniaFirstProcessor++;
											if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
												najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
											else
												najblizszyMaintenanceFirstProcessor = -1;

										// Musismy sprawdziÄ‡ czy uda siÄ™ nam wcisnÄ…Ä‡ nasze zadanie
											if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
												break;
									}

									// Po opuszczeniu pÄ™tli mamy poprawnÄ… wartoĹ›Ä‡ w zmiennej timeFirstProcessor (wystarczy zwiÄ™kszyÄ‡ jÄ… o dĹ‚ugoĹ›Ä‡ zadania)
										timeFirstProcessor += currentTask->duration;

										if(DEBUG)
											debugFile << "I Czas FM " << timeFirstProcessor << endl;

									// Ustawiamy zmiennÄ… czasowÄ… zakoĹ„czenia zadania
										currentTask->endTime = timeFirstProcessor;

									// Zaznaczamy w tablicy pomocniczej ĹĽe czÄ™Ĺ›Ä‡ pierwsza zadania byĹ‚a uĹĽyta
										firstPart[taskID] = true;
								}

								// ZwiÄ™kszamy iloĹ›Ä‡ zadaĹ„ jakie przerobiliĹ›my
									count++;

						} else { // PrzydziaĹ‚ zadania na maszynÄ™ nr 2
							// Sprawdzamy czy zadanie moĹĽna umieĹ›ciÄ‡ przed maintenance najbliĹĽszym (jeĹĽeli jest  on -1 to juĹĽ nie wystÄ…pi)
								if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
									// Ustawiamy czas na maszynie drugiej
										timeSecondProcessor += currentTask->duration;

										if(DEBUG)
											debugFile << "I Czas SM: " << timeSecondProcessor << endl;

									// Ustawiamy czas zakoĹ„czenia zadania
										currentTask->endTime = timeSecondProcessor;

									// Ustawiamy ĹĽe zadanie zostaĹ‚o uĹĽyte (part I)
										firstPart[taskID] = true;

								} else { // Nie umieĹ›ciliĹ›my zadania przed przerwÄ…
									while(true) {
										// Przesuwamy siÄ™ na chwilÄ™ po przerwaniu
											timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;

										// Ustawiamy czas nastÄ™pnego przerwania
											numerPrzerwaniaSecondProcessor++;
											if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
												najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
											else
												najblizszyMaintenanceSecondProcessor = -1;

											if(DEBUG)
												debugFile << "Druga = " << timeSecondProcessor << " oraz " << najblizszyMaintenanceSecondProcessor << endl;

										// Musismy sprawdziÄ‡ czy uda siÄ™ nam wcisnÄ…Ä‡ nasze zadanie
											if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
												break;
									}

									// Po opuszczeniu pÄ™tli mamy poprawnÄ… wartoĹ›Ä‡ w zmiennej timeSecondProcessor(wystarczy zwiÄ™kszyÄ‡ jÄ… o dĹ‚ugoĹ›Ä‡ zadania)
										timeSecondProcessor += currentTask->duration;

										if(DEBUG)
											debugFile << "Czas SM " << timeSecondProcessor << endl;

									// Ustawiamy zmiennÄ… czasowÄ… zakoĹ„czenia zadania
										currentTask->endTime = timeSecondProcessor;

									// Zaznaczamy w tablicy pomocniczej ĹĽe czÄ™Ĺ›Ä‡ pierwsza zadania byĹ‚a uĹĽyta
										firstPart[taskID] = true;
								}

								// ZwiÄ™kszamy iloĹ›Ä‡ zadaĹ„ jakie przerobiliĹ›my
									count++;
						}
				} else {
				// PRZYDZIELAMY DRUGÄ„ CZÄĹšÄ† ZADANIA

					// MogÄ… wystÄ…piÄ‡ problemy z zapÄ™tleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartoĹ›Ä‡
						licznikOdwiedzonych[taskID]++;

					// Sprawdzamy typ zadania - jeĹĽeli jest zero to podstawiamy pod zmiennÄ… pomocniczÄ…
						if(zadaniaLokalne[pozycja]->part == 1) {
							currentTask = zadaniaLokalne[pozycja];
						} else { // JeĹĽeli nie - konieczne jest podstawienie czÄ™Ĺ›ci komplementarnej wylosowanego zadania
							currentTask = zadaniaLokalne[pozycja]->anotherPart;
						}

					// Sprawdzamy typ zadania
						if(currentTask->assigment == 1) { // PrzydziaĹ‚ na drugÄ… maszynÄ™
							// Sprawdzamy czy czas na maszynie nie jest mniejszy od zakoĹ„czenia siÄ™ pierwszej czÄ™Ĺ›ci
							if(timeSecondProcessor < currentTask->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteĹ›my po raz x w pÄ™tli
								if(licznikOdwiedzonych[taskID] >= MIN_TASK_COUNTER) {
									if(DEBUG)
										debugFile << "Przestawiono czas! M1" << endl;
									// Tworzymy pomocniczÄ… zmiennÄ… odlegĹ‚oĹ›ci
										int minTime = INT_MAX;
										int tempTime = 0;

									// Resetujemy liczniki i patrzymy na odlegĹ‚oĹ›ci
										for(int i = 0; i < iloscZadan; i++) {
											licznikOdwiedzonych[i] = 0;

											if(!secondPart[i]) {
												int tempTime = currentTask->anotherPart->endTime - timeSecondProcessor;
												if(tempTime < minTime)
													minTime = tempTime;
											}
										}

									// Przestawiamy czas na maszynie
										timeSecondProcessor += minTime;

								} else // JeĹĽeli nie mamy osiÄ…gniÄ™tej wartoĹ›ci to pomijamy iteracjÄ™
									continue;
							}

							// Zadanie moĹĽna umieĹ›ciÄ‡
								// Sprawdzamy czy zadanie moĹĽna umieĹ›ciÄ‡ przed maintenance najbliĹĽszym (jeĹĽeli jest  on -1 to juĹĽ nie wystÄ…pi)
								if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
									// Ustawiamy czas na maszynie pierwszej
										timeSecondProcessor += currentTask->duration;

									// Ustawiamy czas zakoĹ„czenia zadania
										currentTask->endTime = timeSecondProcessor;

									// Ustawiamy ĹĽe zadanie zostaĹ‚o uĹĽyte (part II)
										secondPart[taskID] = true;

								} else { // Nie umieĹ›ciliĹ›my zadania przed przerwÄ…
									while(true) {
										// Przesuwamy siÄ™ na chwilÄ™ po przerwaniu
											timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;

										// Ustawiamy czas nastÄ™pnego przerwania
											numerPrzerwaniaSecondProcessor++;
											if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
												najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
											else
												najblizszyMaintenanceSecondProcessor = -1;

										// Musismy sprawdziÄ‡ czy uda siÄ™ nam wcisnÄ…Ä‡ nasze zadanie
											if((timeSecondProcessor + currentTask->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
												break;
									}

									// Po opuszczeniu pÄ™tli mamy poprawnÄ… wartoĹ›Ä‡ w zmiennej timeSecondProcessor (wystarczy zwiÄ™kszyÄ‡ jÄ… o dĹ‚ugoĹ›Ä‡ zadania)
										timeSecondProcessor += currentTask->duration;

									// Ustawiamy zmiennÄ… czasowÄ… zakoĹ„czenia zadania
										currentTask->endTime = timeSecondProcessor;

									// Zaznaczamy w tablicy pomocniczej ĹĽe czÄ™Ĺ›Ä‡ pierwsza zadania byĹ‚a uĹĽyta
										secondPart[taskID] = true;
								}

								// ZwiÄ™kszamy iloĹ›Ä‡ zadaĹ„ jakie przerobiliĹ›my
									count++;
						} else {
							// Sprawdzamy czy czas na maszynie nie jest mniejszy od zakoĹ„czenia siÄ™ pierwszej czÄ™Ĺ›ci
							if(timeFirstProcessor < currentTask->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteĹ›my po raz x w pÄ™tli
								if(licznikOdwiedzonych[taskID] >= MIN_TASK_COUNTER) {
									if(DEBUG)
										debugFile << "Przestawiono czas! M0" << endl;

									// Tworzymy pomocniczÄ… zmiennÄ… odlegĹ‚oĹ›ci
										int minTime = INT_MAX;
										int tempTime = 0;

									// Resetujemy liczniki i patrzymy na odlegĹ‚oĹ›ci
										for(int i = 0; i < iloscZadan; i++) {
											licznikOdwiedzonych[i] = 0;

											if(!secondPart[i]) {
												tempTime = currentTask->anotherPart->endTime - timeFirstProcessor;
												if(tempTime < minTime)
													minTime = tempTime;
											}
										}

									// Przestawiamy czas na maszynie
										timeFirstProcessor += minTime;

								} else // JeĹĽeli nie mamy osiÄ…gniÄ™tej wartoĹ›ci to pomijamy iteracjÄ™
									continue;
							}

							// Zadanie moĹĽna umieĹ›ciÄ‡
								// Sprawdzamy czy zadanie moĹĽna umieĹ›ciÄ‡ przed maintenance najbliĹĽszym (jeĹĽeli jest  on -1 to juĹĽ nie wystÄ…pi)
								if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
									// Ustawiamy czas na maszynie pierwszej
										timeFirstProcessor += currentTask->duration;

									// Ustawiamy czas zakoĹ„czenia zadania
										currentTask->endTime = timeFirstProcessor;

									// Ustawiamy ĹĽe zadanie zostaĹ‚o uĹĽyte (part II)
										secondPart[taskID] = true;

								} else { // Nie umieĹ›ciliĹ›my zadania przed przerwÄ…
									while(true) {
										// Przesuwamy siÄ™ na chwilÄ™ po przerwaniu
											timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;

										// Ustawiamy czas nastÄ™pnego przerwania
											numerPrzerwaniaFirstProcessor++;
											if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
												najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
											else
												najblizszyMaintenanceFirstProcessor = -1;

										// Musismy sprawdziÄ‡ czy uda siÄ™ nam wcisnÄ…Ä‡ nasze zadanie
											if((timeFirstProcessor + currentTask->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
												break;
									}

									// Po opuszczeniu pÄ™tli mamy poprawnÄ… wartoĹ›Ä‡ w zmiennej timeSecondProcessor (wystarczy zwiÄ™kszyÄ‡ jÄ… o dĹ‚ugoĹ›Ä‡ zadania)
										timeFirstProcessor += currentTask->duration;

									// Ustawiamy zmiennÄ… czasowÄ… zakoĹ„czenia zadania
										currentTask->endTime = timeFirstProcessor;

									// Zaznaczamy w tablicy pomocniczej ĹĽe czÄ™Ĺ›Ä‡ pierwsza zadania byĹ‚a uĹĽyta
										secondPart[taskID] = true;
								}

								// ZwiÄ™kszamy iloĹ›Ä‡ zadaĹ„ jakie przerobiliĹ›my
									count++;
						}
				}
		}

		// Czyszczenie pamiÄ™ci - zwalnianie niepotrzebnych zasobĂłw
			delete firstPart;
			delete secondPart;
			delete licznikOdwiedzonych;

	return zadaniaLokalne;
}

// Odczyt danych zadaĹ„ na ekran
void OdczytDanychZadan(vector<Task*> &listaZadan) {
	// Przeliczenie iloĹ›ci operacji do zmienne pomocniczej aby nie liczyÄ‡ operacji w kaĹĽdej iteracji
	int size = listaZadan.size();

	// Przesortowanie listy zadaĹ„ aby mieÄ‡ obok siebie zadania z tym samym ID
		SortujZadaniaPoID(listaZadan);

	// PÄ™tla odczytu wartoĹ›ci zadaĹ„
		for(int i = 0; i < size; i++) {
			cout << "--- ID: " << listaZadan[i]->ID << " (Part " << listaZadan[i]->part << ") przydzial: M" << listaZadan[i]->assigment << " duration = " << listaZadan[i]->duration << " --- zakonczenie = " << listaZadan[i]->endTime << " --- " << endl;
		}
}

// Tworzenie timeline dla obserwacji wynikĂłw pracy
void UtworzGraf(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwan, long int wynik, string nameParam) {
	int iloscZadan = listaZadan.size(); // IloĹ›Ä‡ zadaĹ„ w systemie
	int iloscPrzerwan = listaPrzerwan.size(); // IloĹ›Ä‡ okresĂłw przestojĂłw na maszynach

	ofstream file;
	string fileName = "index_" + nameParam + ".html";
	file.open(fileName.c_str());
	file << "<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" /><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
	file << "<title>OK - Wyniki pracy generatora</title></head><body><script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>";
	file << "<script type=\"text/javascript\">google.charts.load(\"current\", {packages:[\"timeline\"]});google.charts.setOnLoadCallback(drawChart);function drawChart() {";
	file << "var container = document.getElementById('example4.2');var chart = new google.visualization.Timeline(container);var dataTable = new google.visualization.DataTable();";
	file << "dataTable.addColumn({ type: 'string', id: 'Role' });dataTable.addColumn({ type: 'string', id: 'Name' });dataTable.addColumn({ type: 'number', id: 'Start' });dataTable.addColumn({ type: 'number', id: 'End' });dataTable.addRows([";

	int timeStart = 0;
	int timeStop = 0;

	// Zapisujemy do pliku nasze zadania
		for(int i = 0; i < iloscZadan; i++) {
			timeStop = listaZadan[i]->endTime;
			timeStart = timeStop - listaZadan[i]->duration;

			file << "[ 'M" << listaZadan[i]->assigment + 1 << "', 'Zadanie " << listaZadan[i]->ID << "', " << timeStart << ", " << timeStop << " ]," << endl;
		}

	// Zapis przerwaĹ„
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

// Obliczanie wartoĹ›ci funkcji celu
long int ObliczFunkcjeCelu(vector<Task*> &lista) {
	int size = lista.size();
	long int sum = 0;

	for(int i = 0; i < size; i++) {
		sum += lista[i]->endTime;
	}

	return sum;
}

// PodziaĹ‚ struktury T na maszyny
template <class T>
void PodzielStrukturyNaMaszyny(vector<T*> &listaWejsciowa, vector<T*> &firstProcessor, vector<T*> &secondProcessor) {
	// Zmienna pomocnicza by skrĂłciÄ‡ czas pracy (nie trzeba x razy liczyÄ‡)
		int size = listaWejsciowa.size();

	//Sprawdzamy do jakiej maszyny przypisana jest struktura
		for(int i = 0; i < size; i++) {
			T * operacja = listaWejsciowa[i];

			if(operacja->assigment == 0) {
				firstProcessor.push_back(operacja);
			} else {
				secondProcessor.push_back(operacja);
			}
		}
}

// Obliczanie dĹ‚ugoĹ›ci Task / Maintenance list
template <class T>
long int ObliczDlugoscOperacji(vector<T*> &lista) {
	int size = lista.size();
	long int sum = 0;

	for(int i = 0; i < size; i++) {
		sum += lista[i]->duration;
	}

	return sum;
}

// Zapis wynikĂłw do pliku tekstowego
void ZapiszWynikiDoPliku(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor, long int firstSolutionValue, int numerInstancjiProblemu, string nameParam) {
	ofstream file;

	string fileName;
	stringstream ss;
	ss << "wyniki/wyniki_" << numerInstancjiProblemu << "_" << nameParam << ".txt";
	ss >> fileName;
	file.open(fileName.c_str());

	if(file.is_open()) {
			long int optimalSolutionValue = ObliczFunkcjeCelu(listaZadan); // WartoĹ›Ä‡ funkcji celu dla rozwiÄ…zania optymalnego
			vector<Task*> taskFirstProcessor, taskSecondProcessor; // Wektory dla podziaĹ‚u zadaĹ„ na maszyny
			int taskFirstProcessorSize; // IloĹ›Ä‡ zadaĹ„ na pierwszym procesorze
			int taskSecondProcessorSize; // IloĹ›Ä‡ zadaĹ„ na drugim procesorze
			int numerPrzerwania = 0; // Numer aktualnie rozpatrywanego przerwania
			int najblizszyMaintenance = -1; // Czas momentu ROZPOCZÄCIA przerwania
			int processorTime = 0; // Czas procesora
			int count = 0; // IloĹ›Ä‡ operacji ktĂłre zostaĹ‚y juĹĽ umieszczone w pliku wynikowym
			int maxCount; // IloĹ›Ä‡ operacji ktĂłre trzeba umieĹ›ciÄ‡ (liczba operacji + przerwania)
			int taskPoint = 0; // Zmienna wskazujÄ…ca aktualnie rozpatrywane zadanie z listy operacji
			int countIldeFirstProcessor = 0; // Licznik okresĂłw bezczynnoĹ›ci dla maszyny pierwszej
			int countIldeSecondProcessor = 0; // Licznik okresĂłw bezczynnoĹ›ci dla maszyny drugiej
			int ildeTimeFirstProcessor = 0; // OgĂłlny czas bezczynnoĹ›ci na maszynie pierwszej
			int ildeTimeSecondProcessor = 0; // OgĂłlny czas bezczynnoĹ›ci na maszynie drugiej

		// Podzielenie listy zadaĹ„ na maszyny i przypisanie iloĹ›ci do zmiennych pomocniczych
			PodzielStrukturyNaMaszyny<Task>(listaZadan, taskFirstProcessor, taskSecondProcessor);
			taskFirstProcessorSize = taskFirstProcessor.size();
			taskSecondProcessorSize = taskSecondProcessor.size();

		// Sortowanie zadaĹ„
			SortujZadaniaPoEndTime(taskFirstProcessor);
			SortujZadaniaPoEndTime(taskSecondProcessor);

		// Przypisanie numeru instancji
			file << "**** " << numerInstancjiProblemu << " ****" << endl;

		// Przypisanie wartoĹ›ci optymalnej oraz wartoĹ›ci poczÄ…tkowej wygenerowanej przez generator losowy
			file << optimalSolutionValue << ", " << firstSolutionValue << endl;

		// Przypisanie do pliku utworzonej instancji
			file << "M1:";

			if(listaPrzerwanFirstProcessor.size() > 0)
				najblizszyMaintenance = listaPrzerwanFirstProcessor[0]->readyTime;
			maxCount = taskFirstProcessorSize + listaPrzerwanFirstProcessor.size(); // maxCount dla pierwszej maszyny
			while(count < maxCount) {

				if(taskPoint >= 0 && processorTime == (taskFirstProcessor[taskPoint]->endTime - taskFirstProcessor[taskPoint]->duration)) {
					// Zapis do pliku
						file << "op" << taskFirstProcessor[taskPoint]->part + 1 << "_" << taskFirstProcessor[taskPoint]->ID << ", " << taskFirstProcessor[taskPoint]->endTime - taskFirstProcessor[taskPoint]->duration
							 << ", " << taskFirstProcessor[taskPoint]->duration << "; ";

					// Przestawienie czasu
						processorTime = taskFirstProcessor[taskPoint]->endTime;

					// Skok do kolejnej wartoĹ›ci
						taskPoint++;

					// Musimy sprawdziÄ‡ czy nie wychodzimy poza zakres
						if(taskPoint >= taskFirstProcessorSize) {
							taskPoint = -1;
						}

					// ZwiÄ™kszamy licznik odwiedzonych operacji
						count++;
				} else if (processorTime == najblizszyMaintenance) {
					// Zapis do pliku
						file << "maint" << numerPrzerwania + 1 << "_M1, " << listaPrzerwanFirstProcessor[numerPrzerwania]->readyTime << ", "
							 << listaPrzerwanFirstProcessor[numerPrzerwania]->duration << "; ";

					// Przestawienie czasu
						processorTime = listaPrzerwanFirstProcessor[numerPrzerwania]->readyTime + listaPrzerwanFirstProcessor[numerPrzerwania]->duration;

					// Konieczne jest sprawdzenie czy nie wychodzimi poza zakres
						numerPrzerwania++;
						if(numerPrzerwania >= listaPrzerwanFirstProcessor.size()) {
							najblizszyMaintenance = -1;
						} else {
							najblizszyMaintenance = listaPrzerwanFirstProcessor[numerPrzerwania]->readyTime;
						}

					// ZwiÄ™kszamy licznik odwiedzonych operacji
						count++;
				} else { // BezczynnoĹ›Ä‡

					// Sprawdzamy ktĂłre zdarzenie bÄ™dzie wczeĹ›niej - wystÄ…pienie zadania czy maintenance
						int minTime = INT_MAX;
						if(taskPoint >= 0) {
							int temp =  taskFirstProcessor[taskPoint]->endTime - taskFirstProcessor[taskPoint]->duration - processorTime;
							if(temp < minTime)
								minTime = temp;
						}
						if(((najblizszyMaintenance - processorTime) < minTime) && najblizszyMaintenance > -1) {
							minTime = najblizszyMaintenance - processorTime;
						}

					// Zapis do pliku danych o bezczynnoĹ›ci
						file << "idle" << countIldeFirstProcessor + 1 << "_M1, " << processorTime << ", " << minTime << "; ";
						countIldeFirstProcessor++;

					// Dodanie do ogĂłlnego licznika bezczynnoĹ›ci zapisanego czasu
						ildeTimeFirstProcessor += minTime;

					// Przestawienie czasu maszyny
						processorTime += minTime;
				}
			}

			file << endl << "M2:";

			// Zerowanie zmiennych uniwersalnych
			taskPoint = 0;
			count = 0;
			processorTime = 0;
			numerPrzerwania = 0;
			if(listaPrzerwanSecondProcessor.size() > 0)
				najblizszyMaintenance = listaPrzerwanSecondProcessor[0]->readyTime;
			else
				najblizszyMaintenance = -1;
			maxCount = taskSecondProcessorSize + listaPrzerwanSecondProcessor.size(); // maxCount dla drugiej maszyny
			while(count < maxCount) {
				if(taskPoint >= 0 && processorTime == (taskSecondProcessor[taskPoint]->endTime - taskSecondProcessor[taskPoint]->duration)) {
					// Zapis do pliku
						file << "op" << taskSecondProcessor[taskPoint]->part + 1 << "_" << taskSecondProcessor[taskPoint]->ID << ", " << taskSecondProcessor[taskPoint]->endTime - taskSecondProcessor[taskPoint]->duration
							 << ", " << taskSecondProcessor[taskPoint]->duration << "; ";

					// Przestawienie czasu
						processorTime = taskSecondProcessor[taskPoint]->endTime;

					// Skok do kolejnej wartoĹ›ci
						taskPoint++;

					// Musimy sprawdziÄ‡ czy nie wychodzimy poza zakres
						if(taskPoint >= taskSecondProcessorSize) {
							taskPoint = -1;
						}

					// ZwiÄ™kszamy licznik odwiedzonych operacji
						count++;
				} else if (processorTime == najblizszyMaintenance) {
					// Zapis do pliku
						file << "maint" << numerPrzerwania + 1 << "_M1, " << listaPrzerwanSecondProcessor[numerPrzerwania]->readyTime << ", "
							 << listaPrzerwanSecondProcessor[numerPrzerwania]->duration << "; ";

					// Przestawienie czasu
						processorTime = listaPrzerwanSecondProcessor[numerPrzerwania]->readyTime + listaPrzerwanSecondProcessor[numerPrzerwania]->duration;

					// Konieczne jest sprawdzenie czy nie wychodzimi poza zakres
						numerPrzerwania++;
						if(numerPrzerwania >= listaPrzerwanSecondProcessor.size()) {
							najblizszyMaintenance = -1;
						} else {
							najblizszyMaintenance = listaPrzerwanSecondProcessor[numerPrzerwania]->readyTime;
						}

					// ZwiÄ™kszamy licznik odwiedzonych operacji
						count++;
				} else { // BezczynnoĹ›Ä‡
					// Sprawdzamy ktĂłre zdarzenie bÄ™dzie wczeĹ›niej - wystÄ…pienie zadania czy maintenance
						int minTime = INT_MAX;
						if(taskPoint >= 0) {
							int temp =  taskSecondProcessor[taskPoint]->endTime - taskSecondProcessor[taskPoint]->duration - processorTime;
							if(temp < minTime)
								minTime = temp;
						}
						if(((najblizszyMaintenance - processorTime) < minTime) && najblizszyMaintenance > -1) {
							minTime = najblizszyMaintenance - processorTime;
						}

					// Zapis do pliku danych o bezczynnoĹ›ci
						file << "ilde" << countIldeSecondProcessor + 1 << "_M2, " << processorTime << ", " << minTime << "; ";

					// Inkrementacja numeru przerwania
						countIldeSecondProcessor++;

					// Dodanie do ogĂłlnego licznika bezczynnoĹ›ci zapisanego czasu
						ildeTimeSecondProcessor += minTime;

					// Przestawienie czasu maszyny
						processorTime += minTime;
				}
			}

			// Dopisanie wartoĹ›ci sum
				file << endl << listaPrzerwanFirstProcessor.size() << ", " << ObliczDlugoscOperacji<Maintenance>(listaPrzerwanFirstProcessor) << endl
					 << listaPrzerwanSecondProcessor.size() << ", " << ObliczDlugoscOperacji<Maintenance>(listaPrzerwanSecondProcessor) << endl
					 << countIldeFirstProcessor << ", " << ildeTimeFirstProcessor << endl
					 << countIldeSecondProcessor << ", " << ildeTimeSecondProcessor << endl << "*** EOF ***";

			// Czyszczenie pamiÄ™ci operacyjnej
				taskFirstProcessor.clear();
				taskSecondProcessor.clear();
	}
    else{
        if(DEBUG) cout<<"Nie utworzono pliku: "<<fileName.c_str()<<endl;
    }
}

// Mutacja jednego rozwiÄ…zania z zaĹ‚oĹĽeniem podzielenia operacji na dwie maszyny
vector<Task*> Mutacja(vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor) {
	// Zmienne operacyjne
		vector<Task*> taskListFirstProcessor, taskListSecondProcessor; // Wektory dla podziaĹ‚u zadaĹ„ na maszyny

	// Podzielenie listy zadaĹ„ na maszyny i przypisanie iloĹ›ci do zmiennych pomocniczych
			PodzielStrukturyNaMaszyny<Task>(listaZadan, taskListFirstProcessor, taskListSecondProcessor);

		int iloscZadan = taskListFirstProcessor.size();
		int firstTaskPosition = 0; // Random - pozycja pierwszego zadania
		int secondTaskPosition = 0;  // Random - pozycja drugiego zadania

		int processor = rand() % 2; // Random - wybĂłr maszyny ktĂłrÄ… dotyczyÄ‡ bÄ™dzie mutacja
		int timeFirstProcessor = 0; // Czas na maszynie pierwszej
		int timeSecondProcessor = 0; // Czas na maszynie drugiej
		Task * currentTaskFirstProcessor = NULL; // Zadanie na maszynie pierwszej
		Task * currentTaskSecondProcessor = NULL; // Zadanie na maszynie drugiej
		int numerPrzerwaniaFirstProcessor = 0; // Numer aktualnego przerwania na procesorze pierwszym
		int numerPrzerwaniaSecondProcessor = 0; // Numer aktualnego przerwania na procesorze drugim
		int countTask = 0; // Licznik sprawdzionych juĹĽ zadaĹ„
		int najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime; // Czas momentu ROZPOCZÄCIA przerwania na procesorze pierwszym
		int najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime; // Czas momentu ROZPOCZÄCIA przerwania na procesorze drugim
		int listaPrzerwanFirstProcessorSize = listaPrzerwanFirstProcessor.size(); // IloĹ›Ä‡ przerwaĹ„ dla pierwszego procesora - aby nie liczyÄ‡ za kaĹĽdym razem tej wartoĹ›ci
		int listaPrzerwanSecondProcessorSize = listaPrzerwanSecondProcessor.size(); // IloĹ›Ä‡ przerwaĹ„ dla drugiej maszyny - podobnie jak wyĹĽej, unikamy niepotrzebnego, wielokrotnego liczenia tej wartoĹ›ci
		int taskIDFirstProcessor = 0; // Numer zadania na maszynie pierwszej
		int taskIDSecondProcessor = 0; // Numer zadania na maszynie drugiej

		int iteratorFP = 0; // Numer rozpatrywanego zadania na maszynie pierwszej
		int iteratorSP = 0; // Numer aktualnie rozpatrywanego zadania na maszynie drugiej

	// Wektory kolejnoĹ›ci zadaĹ„ (ID)
		int *taskOrderFirstProcessor = new int[iloscZadan];
		int *taskOrderSecondProcessor = new int[iloscZadan];

	// Licznik odwiedzonych aby nie zapÄ™tliÄ‡ siÄ™ w czasach bezczynnoĹ›ci
		int *licznikOdwiedzonych = new int[iloscZadan];

	// Pomocnicze tablice part I & part II aby przyspieszyÄ‡ proces sprawdzania
		bool *firstPart = new bool[iloscZadan];
		bool *secondPart = new bool[iloscZadan];

	// Sortowanie zadaĹ„ w listach
		SortujZadaniaPoEndTime(taskListFirstProcessor);
		SortujZadaniaPoEndTime(taskListSecondProcessor);

	// Tworzymy wektor kolejnoĹ›ci zadaĹ„ i zerujemy tablice pomocnicze
		for(int i = 0; i < iloscZadan; i++) {
			taskOrderFirstProcessor[i] = taskListFirstProcessor[i]->ID - 1;
			taskOrderSecondProcessor[i] = taskListSecondProcessor[i]->ID - 1;
			firstPart[i] = false;
			secondPart[i] = false;
			licznikOdwiedzonych[i] = 0;
		}

		if(DEBUG) {
			debugFile << "Przed mutacjÄ…:" << endl;
			for(int i = 0; i < iloscZadan; i++) {
				debugFile << taskOrderFirstProcessor[i] << " | " << taskOrderSecondProcessor[i] << endl;
			}
		}

	// PÄ™tla losowania i zmiany kolejnoĹ›ci zadaĹ„
		while(true) {
			// Losujemy wartoĹ›ci
				firstTaskPosition = (int)(rand() / (RAND_MAX + 1.0) * iloscZadan);
				secondTaskPosition = (int)(rand() / (RAND_MAX + 1.0) * iloscZadan);

			if(processor == 0) { // Przestawienie kolejnoĹ›ci zadaĹ„ dotyczy maszyny pierwszej
				// Sprawdzamy czy te zadania moĹĽemy mutowaÄ‡ (zaĹ‚oĹĽenie - przestawiamy tylko zadania z tym samym wskaĹşnikiem czÄ™Ĺ›ci Part)
					if(secondTaskPosition != firstTaskPosition && taskListFirstProcessor[firstTaskPosition]->part == taskListFirstProcessor[secondTaskPosition]->part) {
					// Zamiana kolejnoĹ›ci zadaĹ„ w liĹ›cie
						int temp = taskOrderFirstProcessor[firstTaskPosition];
						taskOrderFirstProcessor[firstTaskPosition] = taskOrderFirstProcessor[secondTaskPosition];
						taskOrderFirstProcessor[secondTaskPosition] = temp;

						break;
					} else {
						continue; // Skok do kolejnej iteracji i nowego losowania
					}
			} else { // Zmiany kolejnoĹ›ci dla maszynie nr 2
				if(secondTaskPosition != firstTaskPosition && taskListSecondProcessor[firstTaskPosition]->part == taskListSecondProcessor[secondTaskPosition]->part) {
					// Zamiana kolejnoĹ›ci zadaĹ„ w liĹ›cie
						int temp = taskOrderSecondProcessor[firstTaskPosition];
						taskOrderSecondProcessor[firstTaskPosition] = taskOrderSecondProcessor[secondTaskPosition];
						taskOrderSecondProcessor[secondTaskPosition] = temp;

						break;
					} else {
						continue; // Skok do kolejnej iteracji i nowego losowania
					}
			}
		}

		if(DEBUG) {
			debugFile << "PO mutacji:" << endl;
			for(int i = 0; i < iloscZadan; i++) {
				debugFile << taskOrderFirstProcessor[i] << " | " << taskOrderSecondProcessor[i] << endl;
			}
		}

		// Posortowanie zadaĹ„ wedĹ‚ug ID - aby Ĺ‚atwo odwoĹ‚ywaÄ‡ siÄ™ poprzez wartoĹ›Ä‡ z tablicy kolejnoĹ›ci zadaĹ„
			SortujZadaniaPoID(taskListFirstProcessor);
			SortujZadaniaPoID(taskListSecondProcessor);

	// PÄ™tla ustawiajÄ…ca nowe czasy zakoĹ„czenia dla naszych operacji
		while(countTask < iloscZadan*2) {
			// Sprawdzamy czy nie wyskoczyliĹ›my na maszynie pierwszej poza zakres vektora
			if(iteratorFP < iloscZadan) {
				// Przypisujemy zadanie do zmiennej pomocniczej
					taskIDFirstProcessor = taskOrderFirstProcessor[iteratorFP];
					currentTaskFirstProcessor = taskListFirstProcessor[taskIDFirstProcessor];

				// Sprawdzamy part zadania - jeĹĽeli jest to I to moĹĽna wstawiaÄ‡ od razu, jeĹĽeli II trzeba poczekaÄ‡ aĹĽ zostanie wstawiona czÄ™Ĺ›Ä‡ I na maszynie drugiej
					if(currentTaskFirstProcessor->part == 0) {
						// Sprawdzamy czy zadanie uda siÄ™ ustawiÄ‡ przed najblizszym maintenance na maszynie
							if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
								// Ustawiamy czas na maszynie pierwszej
									timeFirstProcessor += currentTaskFirstProcessor->duration;

								if(DEBUG)
									debugFile << "Czas FM: " << timeFirstProcessor << endl;

								// Ustawiamy czas zakoĹ„czenia Part I
									currentTaskFirstProcessor->endTime = timeFirstProcessor;

								// Ustawiamy ĹĽe zadanie zostaĹ‚o uĹĽyte (Part I)
									firstPart[taskIDFirstProcessor] = true;

							} else { // Nie udaĹ‚o siÄ™ umieĹ›ciÄ‡ zadania przed przerwÄ…
								while(true) {
									// Przesuwamy siÄ™ na chwilÄ™ po przerwaniu
										timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;

									// Ustawiamy czas nastÄ™pnego przerwania
										numerPrzerwaniaFirstProcessor++;
										if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
												najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
											else
												najblizszyMaintenanceFirstProcessor = -1;

										// Musismy sprawdziÄ‡ czy uda siÄ™ nam wcisnÄ…Ä‡ nasze zadanie
											if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
												break;
									}

									// Po opuszczeniu pÄ™tli mamy poprawnÄ… wartoĹ›Ä‡ w zmiennej timeFirstProcessor (wystarczy zwiÄ™kszyÄ‡ jÄ… o dĹ‚ugoĹ›Ä‡ zadania)
										timeFirstProcessor += currentTaskFirstProcessor->duration;

										if(DEBUG)
											debugFile << "I Czas FM " << timeFirstProcessor << endl;

									// Ustawiamy zmiennÄ… czasowÄ… zakoĹ„czenia zadania
										currentTaskFirstProcessor->endTime = timeFirstProcessor;

									// Zaznaczamy w tablicy pomocniczej ĹĽe czÄ™Ĺ›Ä‡ pierwsza zadania byĹ‚a uĹĽyta
										firstPart[taskIDFirstProcessor] = true;
							}

							// ZwiÄ™kszamy iloĹ›Ä‡ poprawionych zadaĹ„
								countTask++;

							// Przestawiamy iterator na pierwszej maszynie
								iteratorFP++;

					} else if(firstPart[taskIDFirstProcessor]) { // Sprawdzamy czy zostaĹ‚a wstawiona czÄ™Ĺ›Ä‡ I zadania (ma ono part == 1)
						// MogÄ… wystÄ…piÄ‡ problemy z zapÄ™tleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartoĹ›Ä‡
							licznikOdwiedzonych[taskIDFirstProcessor]++;

							if(timeFirstProcessor < currentTaskFirstProcessor->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteĹ›my po raz x w pÄ™tli
								if(licznikOdwiedzonych[taskIDFirstProcessor] >= MIN_TASK_COUNTER) {
									// Tworzymy pomocniczÄ… zmiennÄ… odlegĹ‚oĹ›ci
										int minTime = INT_MAX;
										int tempTime = 0;

									// Resetujemy liczniki i patrzymy na odlegĹ‚oĹ›ci
										for(int i = 0; i < iloscZadan; i++) {
											licznikOdwiedzonych[i] = 0;

											if(!secondPart[i]) {
												tempTime = currentTaskFirstProcessor->anotherPart->endTime - timeFirstProcessor;
												if(tempTime < minTime)
													minTime = tempTime;
											}
										}

									// Przestawiamy czas na maszynie
										timeFirstProcessor += minTime;

								}
							} else {
								// Zadanie moĹĽna umieĹ›ciÄ‡
								// Sprawdzamy czy zadanie moĹĽna umieĹ›ciÄ‡ przed maintenance najbliĹĽszym (jeĹĽeli jest  on -1 to juĹĽ nie wystÄ…pi)
									if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1)) {
										// Ustawiamy czas na maszynie pierwszej
											timeFirstProcessor += currentTaskFirstProcessor->duration;

										// Ustawiamy czas zakoĹ„czenia zadania
											currentTaskFirstProcessor->endTime = timeFirstProcessor;

										// Przestawiamy iterator oraz iloĹ›Ä‡ zedytowanych zadaĹ„
											iteratorFP++;
											countTask++;

										// Zaznaczamy zadanie jako wykonane w peĹ‚ni
											secondPart[taskIDFirstProcessor] = true;

									} else { // Nie umieĹ›ciliĹ›my zadania przed przerwÄ…
										while(true) {
											// Przesuwamy siÄ™ na chwilÄ™ po przerwaniu
												timeFirstProcessor = najblizszyMaintenanceFirstProcessor + listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->duration;

											// Ustawiamy czas nastÄ™pnego przerwania
												numerPrzerwaniaFirstProcessor++;
												if(numerPrzerwaniaFirstProcessor < listaPrzerwanFirstProcessorSize)
													najblizszyMaintenanceFirstProcessor = listaPrzerwanFirstProcessor[numerPrzerwaniaFirstProcessor]->readyTime;
												else
													najblizszyMaintenanceFirstProcessor = -1;

											// Musismy sprawdziÄ‡ czy uda siÄ™ nam wcisnÄ…Ä‡ nasze zadanie
												if((timeFirstProcessor + currentTaskFirstProcessor->duration) <= najblizszyMaintenanceFirstProcessor || (najblizszyMaintenanceFirstProcessor == -1))
													break;
										}

										// Po opuszczeniu pÄ™tli mamy poprawnÄ… wartoĹ›Ä‡ w zmiennej timeSecondProcessor (wystarczy zwiÄ™kszyÄ‡ jÄ… o dĹ‚ugoĹ›Ä‡ zadania)
											timeFirstProcessor += currentTaskFirstProcessor->duration;

										// Ustawiamy zmiennÄ… czasowÄ… zakoĹ„czenia zadania
											currentTaskFirstProcessor->endTime = timeFirstProcessor;

										// Przestawiamy iterator oraz licznik edycji
											iteratorFP++;
											countTask++;

										// Zaznaczamy zadanie jako wykonane w peĹ‚ni
											secondPart[taskIDFirstProcessor] = true;
									}
							}
					}
			}

			// Zadania na drugim procesorze

			if(iteratorSP < iloscZadan) {
				// Przypisujemy zadanie do zmiennej pomocniczej
					taskIDSecondProcessor = taskOrderSecondProcessor[iteratorSP];
					currentTaskSecondProcessor = taskListSecondProcessor[taskIDSecondProcessor];

				// Sprawdzamy part zadania - jeĹĽeli jest to I to moĹĽna wstawiaÄ‡ od razu, jeĹĽeli II trzeba poczekaÄ‡ aĹĽ zostanie wstawiona czÄ™Ĺ›Ä‡ I na maszynie pierwszej
					if(currentTaskSecondProcessor->part == 0) {
						// Sprawdzamy czy zadanie uda siÄ™ ustawiÄ‡ przed najblizszym maintenance na maszynie
							if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
								// Ustawiamy czas na maszynie pierwszej
									timeSecondProcessor += currentTaskSecondProcessor->duration;

								// Ustawiamy czas zakoĹ„czenia Part I
									currentTaskSecondProcessor->endTime = timeSecondProcessor;

								// Ustawiamy ĹĽe zadanie zostaĹ‚o uĹĽyte (Part I)
									firstPart[taskIDSecondProcessor] = true;

							} else { // Nie udaĹ‚o siÄ™ umieĹ›ciÄ‡ zadania przed przerwÄ…
								while(true) {
									// Przesuwamy siÄ™ na chwilÄ™ po przerwaniu
										timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;

									// Ustawiamy czas nastÄ™pnego przerwania
										numerPrzerwaniaSecondProcessor++;
										if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
												najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
											else
												najblizszyMaintenanceSecondProcessor = -1;

										// Musismy sprawdziÄ‡ czy uda siÄ™ nam wcisnÄ…Ä‡ nasze zadanie
											if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
												break;
									}

									// Po opuszczeniu pÄ™tli mamy poprawnÄ… wartoĹ›Ä‡ w zmiennej timeSecondProcessor (wystarczy zwiÄ™kszyÄ‡ jÄ… o dĹ‚ugoĹ›Ä‡ zadania)
										timeSecondProcessor += currentTaskSecondProcessor->duration;

									// Ustawiamy zmiennÄ… czasowÄ… zakoĹ„czenia zadania
										currentTaskSecondProcessor->endTime = timeSecondProcessor;

									// Zaznaczamy w tablicy pomocniczej ĹĽe czÄ™Ĺ›Ä‡ pierwsza zadania byĹ‚a uĹĽyta
										firstPart[taskIDSecondProcessor] = true;
							}

							// ZwiÄ™kszamy iloĹ›Ä‡ poprawionych zadaĹ„
								countTask++;

							// Przestawiamy iterator na pierwszej maszynie
								iteratorSP++;

					} else if(firstPart[taskIDSecondProcessor]) { // Sprawdzamy czy zostaĹ‚a wstawiona czÄ™Ĺ›Ä‡ I zadania (ma ono part == 1)
						// MogÄ… wystÄ…piÄ‡ problemy z zapÄ™tleniami = dlatego jest dodatkowe zabezpieczenie w postaci liczenia ile razy odwiedzamy wartoĹ›Ä‡
							licznikOdwiedzonych[taskIDSecondProcessor]++;

							if(timeSecondProcessor < currentTaskSecondProcessor->anotherPart->endTime) {
								// Sprawdzamy czy nie jesteĹ›my po raz x w pÄ™tli
								if(licznikOdwiedzonych[taskIDSecondProcessor] >= MIN_TASK_COUNTER) {
									// Tworzymy pomocniczÄ… zmiennÄ… odlegĹ‚oĹ›ci
										int minTime = INT_MAX;
										int tempTime = 0;

									// Resetujemy liczniki i patrzymy na odlegĹ‚oĹ›ci
										for(int i = 0; i < iloscZadan; i++) {
											licznikOdwiedzonych[i] = 0;

											if(!secondPart[i]) {
												tempTime = currentTaskSecondProcessor->anotherPart->endTime - timeSecondProcessor;
												if(tempTime < minTime)
													minTime = tempTime;
											}
										}

									// Przestawiamy czas na maszynie
										timeSecondProcessor += minTime;

								}
							} else {
								// Zadanie moĹĽna umieĹ›ciÄ‡
								// Sprawdzamy czy zadanie moĹĽna umieĹ›ciÄ‡ przed maintenance najbliĹĽszym (jeĹĽeli jest  on -1 to juĹĽ nie wystÄ…pi)
									if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1)) {
										// Ustawiamy czas na maszynie pierwszej
											timeSecondProcessor += currentTaskSecondProcessor->duration;

										// Ustawiamy czas zakoĹ„czenia zadania
											currentTaskSecondProcessor->endTime = timeSecondProcessor;

										// Przestawiamy iterator oraz iloĹ›Ä‡ zedytowanych zadaĹ„
											iteratorSP++;
											countTask++;

										// Zaznaczamy zadanie jako wykonane w peĹ‚ni
											secondPart[taskIDSecondProcessor] = true;

									} else { // Nie umieĹ›ciliĹ›my zadania przed przerwÄ…
										while(true) {
											// Przesuwamy siÄ™ na chwilÄ™ po przerwaniu
												timeSecondProcessor = najblizszyMaintenanceSecondProcessor + listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->duration;

											// Ustawiamy czas nastÄ™pnego przerwania
												numerPrzerwaniaSecondProcessor++;
												if(numerPrzerwaniaSecondProcessor < listaPrzerwanSecondProcessorSize)
													najblizszyMaintenanceSecondProcessor = listaPrzerwanSecondProcessor[numerPrzerwaniaSecondProcessor]->readyTime;
												else
													najblizszyMaintenanceSecondProcessor = -1;

											// Musismy sprawdziÄ‡ czy uda siÄ™ nam wcisnÄ…Ä‡ nasze zadanie
												if((timeSecondProcessor + currentTaskSecondProcessor->duration) <= najblizszyMaintenanceSecondProcessor || (najblizszyMaintenanceSecondProcessor == -1))
													break;
										}

										// Po opuszczeniu pÄ™tli mamy poprawnÄ… wartoĹ›Ä‡ w zmiennej timeSecondProcessor (wystarczy zwiÄ™kszyÄ‡ jÄ… o dĹ‚ugoĹ›Ä‡ zadania)
											timeSecondProcessor += currentTaskSecondProcessor->duration;

										// Ustawiamy zmiennÄ… czasowÄ… zakoĹ„czenia zadania
											currentTaskSecondProcessor->endTime = timeSecondProcessor;

										// Przestawiamy iterator oraz licznik edycji
											iteratorSP++;
											countTask++;

										// Zaznaczamy zadanie jako wykonane w peĹ‚ni
											secondPart[taskIDSecondProcessor] = true;
									}
							}
					}
			}
		}

	// Dopisanie zadaĹ„ ze zmienionymi wartoĹ›ciami
		for(int i = 0; i < iloscZadan; i++) {
			taskListFirstProcessor.push_back(taskListSecondProcessor[i]);
		}

	// Czyszczenie pamiÄ™ci
		delete firstPart;
		delete secondPart;
		delete licznikOdwiedzonych;

	return taskListFirstProcessor;
}

void Turniej(vector< vector<Task*> > &solutionsList) {
	// Przeliczenie rozmiaru otrzymanej struktury listy rozwiÄ…zaĹ„
		int size = solutionsList.size();

	// Utworzenie struktry pomocniczej = tabeli przegranych oraz tabeli z wartoĹ›ciami funkcji celu
		int *solutionsValue = new int[size];
		bool *looserSolution = new bool[size];

	// UzupeĹ‚niami wartoĹ›ci w tabelach
		for(int i = 0; i < size; i++) {
			looserSolution[i] = false;
			solutionsValue[i] = ObliczFunkcjeCelu(solutionsList[i]);
		}

	// Turniej - wracamy do iloĹ›ci rozwiÄ…zaĹ„ jakie chcemy wygenerowaÄ‡
		int toKill = size - MAX_SOLUTIONS;
		int first, second;

		if(DEBUG)
			debugFile << "Kill = " << toKill << endl;

	// PÄ™tla operacyjna
		while(toKill > 0) {
			first = (int)(rand() / (RAND_MAX + 1.0) * size);
			second = (int)(rand() / (RAND_MAX + 1.0) * size);

			if(DEBUG)
				debugFile << "First = " << first << " second =" << second << endl;

			if(first != second && !looserSolution[first] && !looserSolution[second]) {
				// Sprawdzamy ktĂłre z rozwiÄ…zaĹ„ ma mniejszÄ… wartoĹ›Ä‡ funkcji celu
				if(solutionsValue[first] < solutionsValue[second])
					looserSolution[second] = true;
				else
					looserSolution[first] = true;
				toKill--;
			} else
				continue; // Ponawiamy iteracjÄ™ - albo to samo zadanie, albo wylosowano rozwiÄ…zanie ktĂłre odpadĹ‚o
		}

	// UsuniÄ™cie wykluczonych rozwiÄ…zaĹ„
		for(int i = size - 1; i >= 0; i--) {
			if(looserSolution[i]) {
				solutionsList.erase(solutionsList.begin() + i);
			}
		}

	// Czyszczenie pamiÄ™ci operacyjnej
		delete looserSolution;
		delete solutionsValue;
}

void KopiujDaneOperacji(vector<Task*> &listaWejsciowa, vector<Task*> &listaWyjsciowa) {
	// Zmienna pomocnicza by skrĂłciÄ‡ czas pracy (nie trzeba x razy liczyÄ‡)
		int size = listaWejsciowa.size();

		SortujZadaniaPoID(listaWejsciowa);

	//Sprawdzamy do jakiej maszyny przypisana jest struktura
		for(int i = 0; i < size; i += 2) {
			Task *operacja = new Task;
			Task *operacjaDruga = new Task;
			operacja->ID = listaWejsciowa[i]->ID;
			operacja->assigment = listaWejsciowa[i]->assigment;
			operacja->duration = listaWejsciowa[i]->duration;
			operacja->endTime = listaWejsciowa[i]->endTime;
			operacja->part = listaWejsciowa[i]->part;
			operacja->anotherPart = operacjaDruga;

			operacjaDruga->anotherPart = operacja;
			operacjaDruga->ID = listaWejsciowa[i]->anotherPart->ID;
			operacjaDruga->assigment = listaWejsciowa[i]->anotherPart->assigment;
			operacjaDruga->duration = listaWejsciowa[i]->anotherPart->duration;
			operacjaDruga->endTime = listaWejsciowa[i]->anotherPart->endTime;
			operacjaDruga->part = listaWejsciowa[i]->anotherPart->part;

			listaWyjsciowa.push_back(operacja);
			listaWyjsciowa.push_back(operacjaDruga);
		}
}

// GĹ‚Ăłwna pÄ™tla metaheurestyki
vector <Task*> ZnajdzNajlepszeRozwiazanie (vector< vector < Task*> > &listaRozwiazan){
    int sizeListyRozwiazan = listaRozwiazan.size(); // optymalizacja
    int minFunkcjiCelu = INT_MAX; // poczatkowy warunek
    vector <Task*> najlepszeRozwiazanie; // zmienna pomocnicza do zwrocenia
    for(int i=0;i<sizeListyRozwiazan;i++){ // w petli oblicza minimum funkcji
        if(ObliczFunkcjeCelu(listaRozwiazan[i]) < minFunkcjiCelu) {
                najlepszeRozwiazanie = listaRozwiazan[i];
                minFunkcjiCelu = ObliczFunkcjeCelu(najlepszeRozwiazanie);
        }
    }
    return najlepszeRozwiazanie;
}

// dodaje do macierzy fermonow wartosci procentowo po turnieju
void DodajDoMacierzyFermonow(vector< vector<Task*> > &listaRozwiazan, int tablicaWartosciFunkcjiCelu[]){
    int sizeListyRozwiazan = listaRozwiazan.size(); //optymalizacja
    double prawdFunkcjiCelu[sizeListyRozwiazan]; // tablica wartosci funkcji celu (prawdopodobienstw)
    double sumaFunkcjiCelu=0.0; // suma wartosci funkcji celu - do wyznaczenia prawdopodobienstw
    for(int i=0;i<sizeListyRozwiazan;i++){
        prawdFunkcjiCelu[i]=(double)tablicaWartosciFunkcjiCelu[i]; // uzupelniamy tablice wartosciami f celu
        sumaFunkcjiCelu+=prawdFunkcjiCelu[i]; // liczymy sume
    }

    for(int i=0;i<sizeListyRozwiazan;i++){
        prawdFunkcjiCelu[i]=sumaFunkcjiCelu/prawdFunkcjiCelu[i]; // przeksztalcamy w prawdopodobienstwo
    }
    for(int i=0;i<sizeListyRozwiazan;i++){ // dla kazdego rozwiazania
        int sizeRozwiazania=listaRozwiazan[i].size(); // optymalizacja
        for(int j=1;j<sizeRozwiazania;j++){ // dla kazdego zadania (operacji)
            macierzFermonowa[listaRozwiazan[i][j-1]->ID-1][listaRozwiazan[i][j]->ID-1]+=prawdFunkcjiCelu[i];
        }
    }
}
// zanika slady fermonowe co iteracje
void zanikMacierzFermonowa(){
    for(int i=0;i<INSTANCE_SIZE;i++){
        for(int j=0;j<INSTANCE_SIZE;j++){
            macierzFermonowa[i][j]*=(double)(100-PROCENT_ZANIKANIA)/100;
        }
    }
}

// wypisuje macierz fermonowa
void wypiszMacierzFermonowa(){
    for(int i=0;i<INSTANCE_SIZE;i++){
        for(int j=0;j<INSTANCE_SIZE;j++){
            cout<<" ";
            cout.width(7);
            cout<<macierzFermonowa[i][j];
        }
        cout<<endl;
    }
    cout<<endl;
}
void utworzTabliceFunkcjiCelu(vector< vector <Task*> > &listarozwiazan, int tablica[]){
    int sizeListyRozwiazan = listarozwiazan.size();
    for(int i=0;i<sizeListyRozwiazan;i++){
        tablica[i]=ObliczFunkcjeCelu(listarozwiazan[i]);
    }
}

// funckja do splaszczania wiersza w macierzy fermonow (moze ten pow() to nie najlepsza funckja ale innej nie wymyslilem
void FunckjaSplaszczajace(int wiersz){
    for(int i=0;i<INSTANCE_SIZE;i++){
        macierzFermonowa[wiersz][i]=pow(macierzFermonowa[wiersz][i]*INSTANCE_SIZE,1/WYKLADNIK_POTEGI);
    }
}
// GĹ‚Ăłwna pÄ™tla metaheurestyki
void GlownaPetlaMety (vector<Task*> &listaZadan, vector<Maintenance*> &listaPrzerwanFirstProcessor, vector<Maintenance*> &listaPrzerwanSecondProcessor, int numerInstancjiProblemu){
    clockid_t czasStart = clock(); // czas startu mety
    int numerIteracji = 0;
    int aktualnyWiersz =0; // dla funckji splaszczajacej
    vector <Task*> najlepszeRozwiazanie;
    vector < vector <Task*> > listaRozwiazan; // vector ze wszystkimi aktualnymi rozwiazaniami
     vector <Task*> tempTask;
    while ((clock()-czasStart)<MAX_DURATION_PROGRAM_TIME*CLOCKS_PER_SEC){ // warunek by meta nie dziaĹ‚aĹ‚a dĹ‚uĹĽej niz MAX_DURATION_PROGRAM_TIME
        numerIteracji++;

        for(int i=0;i<MAX_SOLUTIONS;){
            if((rand()+1.0)<RAND_MAX*PROBABILTY_OF_RANDOM_GENERATION/100){ // tworzy totalnie losowe z prawdopodobienstwem
            tempTask=GeneratorLosowy(listaZadan,listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor);
            if(numerIteracji==1 && i==0) {// pierwsze napotkane rozwiazanie jest najlepszym
                    KopiujDaneOperacji(tempTask,najlepszeRozwiazanie);
            }
            listaRozwiazan.push_back(tempTask);
            i++; // dodanie rozwiazania do puli
            // zapis do pliku wszystkich wygenerowanch losowa rozwizan dla sprawdzenia czy sie robia
            /*
            string nazwa = "ITER_" + to_string(numerIteracji) + "_LOS_" + to_string(i);
            ZapiszWynikiDoPliku(tempTask,listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor, firstSolutionValue,numerInstancjiProblemu,nazwa);
            */
            tempTask.clear(); // czyszczenie vectora by nie bylo problemow
            }
            else { // tworzy za pomoca tablicy fermonow
                ///TODO
            }
        }
        // zrobienie mutacji

        for(int i=0;i<MAX_SOLUTION_AFTER_MUTATION-MAX_SOLUTIONS;i++){ // musimy dodac roznice tych wartosci
            tempTask = Mutacja(listaRozwiazan[i%MAX_SOLUTIONS],listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor);
            // zapis do pliku wszystkich mutacji dla sprawdzenia czy sie robia
            /*
            string nazwa = "ITER_" + to_string(numerIteracji) + "_MUT_" + to_string(i);
            ZapiszWynikiDoPliku(tempTask,listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor, firstSolutionValue,numerInstancjiProblemu,nazwa);
            */
            listaRozwiazan.push_back(tempTask);
            tempTask.clear();
        }
        // turniej
        Turniej(listaRozwiazan);

        // sortowanie po czasie zakonczenia aby bylo bez problemu w macierzy fermonow i tabeli wartosci funkcji celu
        SortujListeZadanPoEndTime(listaRozwiazan);

        // tworzy tablice funkcji celu
        int tablicaWartosciFunkcjiCelu[listaRozwiazan.size()];
        utworzTabliceFunkcjiCelu(listaRozwiazan,tablicaWartosciFunkcjiCelu);

        // uzupelnia macierz fermonowa
        DodajDoMacierzyFermonow(listaRozwiazan,tablicaWartosciFunkcjiCelu);

        // zanika slad fermonowy
        zanikMacierzFermonowa();

        //funkcja splaszczajaca
        if(!(numerIteracji%CO_ILE_ITERACJI_WIERSZ)){
                FunckjaSplaszczajace(aktualnyWiersz);
                aktualnyWiersz=(aktualnyWiersz+1)%INSTANCE_SIZE; //wylicza aktualny wiersz do funkcji
        }

        // zapis do pliku rozwiazan ktore przeszly turniej (po to by sprawdzic czy najlepsze rozwiazanie aktualnie to jest najlepsze
        for(int i=0;i<listaRozwiazan.size();i++){
            string nazwa = "ITER_" + to_string(numerIteracji) + "_TUR_" + to_string(i);
            ZapiszWynikiDoPliku(listaRozwiazan[i],listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor, firstSolutionValue,numerInstancjiProblemu,nazwa);
        }

        // zapamietywanie najlepszego rozwiazania
        tempTask = ZnajdzNajlepszeRozwiazanie(listaRozwiazan); // najlepsze z tej iteracji
       if(ObliczFunkcjeCelu(tempTask)<ObliczFunkcjeCelu(najlepszeRozwiazanie)) { // porownanie z najlepszym globalnie
            najlepszeRozwiazanie.clear(); // czyszcze dla pewnosci
            KopiujDaneOperacji(tempTask,najlepszeRozwiazanie); // kopia do najlepszego
       }
       tempTask.clear();
    }
    // zapisz najlepszego rozwiazania do pliku
    string nazwa = "NAJLEPSZE";
    ZapiszWynikiDoPliku(najlepszeRozwiazanie,listaPrzerwanFirstProcessor,listaPrzerwanSecondProcessor, firstSolutionValue,numerInstancjiProblemu,nazwa);

}
int main() {
	srand(time(NULL)); // Ino roz reset

	debugFile.open("debug.txt");
	int rozmiarInstancji = INSTANCE_SIZE;
	int numerInstancjiProblemu = INSTANCE_NUMBER;

	// Utworzenie wektora na n zadaĹ„
		vector<Task*> zadania;

	// Wektor przerwaĹ„ pracy na maszynach
		vector<Maintenance*> listaPrzerwan;

	// Wygenerowanie zadaĹ„
		GeneratorInstancji(zadania, rozmiarInstancji, LOWER_TIME_TASK_LIMIT, UPPER_TIME_TASK_LIMIT);

	// Wygenerowanie przerwaĹ„
		GeneratorPrzestojow(listaPrzerwan, MAINTENANCE_FIRST_PROCESSOR, MAINTENANCE_SECOND_PROCESSOR, LOWER_TIME_MAINTENANCE_LIMIT, UPPER_TIME_MAINTENANCE_LIMIT, LOWER_READY_TIME_MAINTENANCE_LIMIT, UPPER_READY_TIME_MAINTENANCE_LIMIT);
//		OdczytPrzerwan(listaPrzerwan);

	// Zapis danych do pliku
		string nameParam;
		stringstream ss;
    	ss << numerInstancjiProblemu;
    	ss >> nameParam; // Parametr przez stringstream, funkcja to_string odmĂłwiĹ‚a posĹ‚uszeĹ„stwa
		ZapiszInstancjeDoPliku(zadania, listaPrzerwan, numerInstancjiProblemu, nameParam);

	// Wczytanie danych z pliku
//		WczytajDaneZPliku(listaZadan, listaPrzerwan, numerInstancjiProblemu, nameParam);

		vector<Maintenance*> przerwaniaFirstProcessor;
		vector<Maintenance*> przerwaniaSecondProcessor;
		SortujPrzerwania(listaPrzerwan);
		PodzielStrukturyNaMaszyny<Maintenance>(listaPrzerwan, przerwaniaFirstProcessor, przerwaniaSecondProcessor);

		vector<Task*> listaZadan;
		listaZadan = GeneratorLosowy(zadania, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
//		OdczytDanychZadan(listaZadan);

		ZapiszWynikiDoPliku(listaZadan, przerwaniaFirstProcessor, przerwaniaSecondProcessor, firstSolutionValue, numerInstancjiProblemu, nameParam);

		long int wynik = ObliczFunkcjeCelu(listaZadan);
//		OdczytPrzerwan(listaPrzerwan);
//		OdczytDanychZadan(listaZadan);
		UtworzGraf(listaZadan, listaPrzerwan, wynik, nameParam);
		nameParam += "w";

		vector<Task*> nowe = GeneratorLosowy(zadania, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
		nowe = Mutacja(nowe, przerwaniaFirstProcessor, przerwaniaSecondProcessor);
//		OdczytDanychZadan(listaZadan);
		vector< vector<Task*> > solution;


		solution.push_back(nowe);
		solution.push_back(listaZadan);
		cout << "S1 " << ObliczFunkcjeCelu(nowe) << endl;
		OdczytDanychZadan(nowe);
		cout << "S2 " << ObliczFunkcjeCelu(listaZadan) << endl;
		OdczytDanychZadan(listaZadan);
		UtworzGraf(nowe, listaPrzerwan, wynik, nameParam);

        //GlownaPetlaMety
        GlownaPetlaMety(zadania,przerwaniaFirstProcessor,przerwaniaSecondProcessor,numerInstancjiProblemu);
		Turniej(solution);

		for(int i = 0; i < solution.size(); i++) {
			cout << "Zapis dla i = " << i << endl;
			long int wyn = ObliczFunkcjeCelu(solution[i]);
			string newNameParam;
			stringstream ss;
			ss << nameParam << "_" << i + 1;
			ss >> newNameParam;
			UtworzGraf(solution[i], listaPrzerwan, wyn, newNameParam);
		}

	// Czyszczenie pamiÄ™ci - zwalnianie niepotrzebnych zasobĂłw
		przerwaniaFirstProcessor.clear();
		przerwaniaSecondProcessor.clear();
		listaPrzerwan.clear();
		listaZadan.clear();

	return 0;
}