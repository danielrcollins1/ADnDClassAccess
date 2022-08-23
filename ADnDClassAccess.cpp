/*
	Name: ADnDClassAccess
	Copyright: 2022
	Author: Daniel R. Collins
	Date: 22/08/22 23:40
	Description: Find odds to generate any class
		from 1E defined stat-generation methods.

		Assumes that ability limits for core classes 
		DO carry to subclasses (unless otherwise noted).
*/
#include <ctime>
#include <iomanip>
#include <iostream>
using namespace std;

// Constants
const int NUM_STATS = 6;
const int DIE_SIDES = 6;
const int NAME_LEN = 16;
const int NUM_TRIALS = 10000;

// Typedef for stat array
typedef int StatArray[NUM_STATS];

// Structure for class record
struct ClassRecord {
	char name[NAME_LEN];
	StatArray statArray;
};

// Class records for minimum abilities
// Order: Str, Int, Wis, Dex, Con, Cha.
ClassRecord classReqs[] = {
	{"Cleric",      { 6,  6,  9,  3,  6,  6}},	
	{"Druid",       { 6,  6, 12,  3,  6, 15}},	
	{"Fighter",     { 9,  3,  6,  6,  7,  6}},	
	{"Paladin",     {12,  9, 13,  6,  9, 17}},	
	{"Ranger",      {13, 13, 14,  6, 14,  6}},	
	{"Magic-User",  { 3,  9,  6,  6,  6,  6}},	
	{"Illusionist", { 3, 15,  6, 16,  3,  6}},	
	{"Thief",       { 6,  6,  3,  9,  6,  6}},	
	{"Assassin",    {12, 11,  3, 12,  6,  3}},	
	{"Monk",        {15,  6, 15, 15, 11,  6}},	
};

// Constant for number of class records
const int NUM_CLASSES = sizeof(classReqs) / sizeof(ClassRecord);

// Class records with abilities sorted descending
ClassRecord classReqsSorted[NUM_CLASSES];

// Bubble sort descending edited from Gaddis C++
void sortArray(int array[], int size) {
   bool swap;
   int temp;

   do {
      swap = false;
      for (int count = 0; count < size - 1; count++) {
         if (array[count] < array[count + 1]) {
            temp = array[count];
            array[count] = array[count + 1];
            array[count + 1] = temp;
            swap = true;
         }
      }
   } 
   while (swap);
}

// Initialize the sort class records
void initClassReqsSorted() {
	for (int i = 0; i < NUM_CLASSES; i++) {
		classReqsSorted[i] = classReqs[i];
		sortArray(classReqsSorted[i].statArray, NUM_STATS);
	}
}

// Roll 1d6
int rollDie() {
	return rand() % DIE_SIDES + 1;
}

// Roll 3d6 (Method 0)
int roll3d6() {
	int sum = 0;
	for (int i = 0; i < 3; i++) {
		sum += rollDie();	
	}
	return sum;
}

// Roll 4d6 drop lowest (Method I)
int roll4d6Drop1() {
	int sum = 0;
	int lowest = 999;
	for (int i = 0; i < 4; i++) {
		int die = rollDie();
		sum += die;
		if (die < lowest) {
			lowest = die;
		}
	}
	sum -= lowest;
	return sum;
}

// Roll 3d6 times 6, take best (Method III)
int roll3d6BestOf6() {
	int highest = 0;
	for (int i = 0; i < 6; i++) {
		int roll = roll3d6();
		if (roll > highest) {
			highest = roll;	
		}
	}
	return highest;
}

// Do we qualify for this class with these stats?
bool classAllowed (ClassRecord classRec, StatArray stats) {
	for (int i = 0; i < NUM_STATS; i++) {
		if (stats[i] < classRec.statArray[i])
			return false;
	}
	return true;
}

// Print class records sorted version for testing.
void printClassReqs() {
	cout << "# Class Requirements #" << endl;
	for (int i = 0; i < NUM_CLASSES; i++) {
		cout << left << setw(NAME_LEN) << classReqs[i].name;
		for (int j = 0; j < NUM_STATS; j++) {
			cout << right << setw(3) << classReqs[i].statArray[j];
		}
		cout << endl;
	}
	cout << endl;
}

// Print sorted class records sorted version for testing.
void printClassReqsSorted() {
	cout << "# Class Requirements Sorted #" << endl;
	for (int i = 0; i < NUM_CLASSES; i++) {
		cout << left << setw(NAME_LEN) << classReqsSorted[i].name;
		for (int j = 0; j < NUM_STATS; j++) {
			cout << right << setw(3) << classReqsSorted[i].statArray[j];
		}
		cout << endl;
	}
	cout << endl;
}

// Print results for a generation method
void printResults(int passCount[NUM_CLASSES]) {
	for (int i = 0; i < NUM_CLASSES; i++) {
		double percent = (double) passCount[i] / NUM_TRIALS * 100;
		cout << left << setw(16) << classReqs[i].name;
		cout << right << setw(6) << percent << " %" << endl;		
	}
	cout << endl;
}

// Make stat record per Method I (to be reordered)
void makeStatsMethodI (StatArray &stats) {
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = roll4d6Drop1();
	}
}

// Count passing for Method I (note reordering)
void countPassingMethodI (int passCount[NUM_CLASSES]) {
	for (int i = 0; i < NUM_CLASSES; i++) {
		passCount[i] = 0;	
	}
	for (int t = 0; t < NUM_TRIALS; t++) {
		StatArray stats;
		makeStatsMethodI(stats);
		sortArray(stats, NUM_STATS);
		for (int i = 0; i < NUM_CLASSES; i++) {
			if (classAllowed(classReqsSorted[i], stats)) {
				passCount[i]++;	
			}
		}
	}
}

// Test Method I
void testMethodI() {
	cout << "# Method I #" << endl;
	int passCount[NUM_CLASSES]; 	
	countPassingMethodI(passCount);
	printResults(passCount);	
	cout << endl;
}

// Make stat record per Method II (to be reordered)
void makeStatsMethodII (StatArray &stats) {
	const int NUM_ROLLS = 12;
	int dozenStats[NUM_ROLLS];
	for (int i = 0; i < NUM_ROLLS; i++) {
		dozenStats[i] = roll3d6();
	}
	sortArray(dozenStats, NUM_ROLLS);
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = dozenStats[i];	
	}
}

// Count passing for Method II (note reordering)
void countPassingMethodII (int passCount[NUM_CLASSES]) {
	for (int i = 0; i < NUM_CLASSES; i++) {
		passCount[i] = 0;	
	}
	for (int t = 0; t < NUM_TRIALS; t++) {
		StatArray stats;
		makeStatsMethodII(stats);
		for (int i = 0; i < NUM_CLASSES; i++) {
			if (classAllowed(classReqsSorted[i], stats)) {
				passCount[i]++;	
			}
		}
	}
}

// Test Method II
void testMethodII() {
	cout << "# Method II #" << endl;
	int passCount[NUM_CLASSES]; 	
	countPassingMethodII(passCount);
	printResults(passCount);	
	cout << endl;
}

// Make stat record per Method III
void makeStatsMethodIII (StatArray &stats) {
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = roll3d6BestOf6();
	}
}

// Count passing for Method III (unsorted)
void countPassingMethodIII (int passCount[NUM_CLASSES]) {
	for (int i = 0; i < NUM_CLASSES; i++) {
		passCount[i] = 0;	
	}
	for (int t = 0; t < NUM_TRIALS; t++) {
		StatArray stats;
		makeStatsMethodIII(stats);
		for (int i = 0; i < NUM_CLASSES; i++) {
			if (classAllowed(classReqs[i], stats)) {
				passCount[i]++;	
			}
		}
	}
}

// Test Method III
void testMethodIII() {
	cout << "# Method III #" << endl;
	int passCount[NUM_CLASSES]; 	
	countPassingMethodIII(passCount);
	printResults(passCount);	
	cout << endl;
}

// Make on stat record per Method IV
void makeStatsMethodIV (StatArray &stats) {
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = roll3d6();
	}
}

// Count passing for Method IV
void countPassingMethodIV (int passCount[NUM_CLASSES]) {
	for (int i = 0; i < NUM_CLASSES; i++) {
		passCount[i] = 0;	
	}
	for (int t = 0; t < NUM_TRIALS; t++) {
		const int NUM_CHARS = 12;
		StatArray stats[NUM_CHARS];
		for (int c = 0; c < NUM_CHARS; c++) {
			makeStatsMethodIV(stats[c]);
		}
		for (int i = 0; i < NUM_CLASSES; i++) {
			for (int c = 0; c < NUM_CHARS; c++) {
				if (classAllowed(classReqs[i], stats[c])) {
					passCount[i]++;	
					break;
				}
			}
		}
	}
}

// Test Method IV
void testMethodIV() {
	cout << "# Method IV #" << endl;
	int passCount[NUM_CLASSES]; 	
	countPassingMethodIV(passCount);
	printResults(passCount);	
	cout << endl;
}

// Main test driver
int main(int argc, char** argv) {
	srand(time(0));
	initClassReqsSorted();
	testMethodI();
	testMethodII();
	testMethodIII();
	testMethodIV();
	return 0;
}

/*
	TODO:
	- Fix output decimal places
	- Spot hand-check some numbers
	- Upload to Github
	- Refactor common stuff (in testMethodX, 
	  hand off pointer to hanlding methods?)
	- Add Method V
	- Add some OD&D, UA style classes
	- Add Bards?
*/
