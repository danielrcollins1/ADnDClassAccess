/*
	Name: ADnDClassAccess
	Copyright: 2022
	Author: Daniel R. Collins
	Date: 22/08/22 23:40
	Description: 
		Find odds to generate any class
		from 1E AD&D defined stat-generation methods.

		Assumes that ability limits for core classes 
		DO carry to subclasses (unless otherwise noted).

		Method Roman numeral identifiers written 
		as Arabic to synch with C++, array indexing, etc.
*/
#include <ctime>
#include <cassert>
#include <iomanip>
#include <iostream>
using namespace std;

// Constants
const int DIE_SIDES = 6;
const int NUM_STATS = 6;
const int NAME_LEN = 16;
const int NUM_METHODS = 5;
const int NUM_TRIALS = 10000;

// Typedef for a stat block
typedef int StatBlock[NUM_STATS];

// Structure for class record
struct ClassRecord {
	char name[NAME_LEN];
	StatBlock statBlock;
};

// Class records for minimum abilities
// Order: Str, Int, Wis, Dex, Con, Cha.
const ClassRecord CLASS_REQS[] = {
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
const int NUM_CLASSES = sizeof(CLASS_REQS) / sizeof(ClassRecord);

// Class records with abilities sorted descending
ClassRecord CLASS_REQS_SORTED[NUM_CLASSES];

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

// Initialize the sorted class records
void initClassReqsSorted() {
	for (int i = 0; i < NUM_CLASSES; i++) {
		CLASS_REQS_SORTED[i] = CLASS_REQS[i];
		sortArray(CLASS_REQS_SORTED[i].statBlock, NUM_STATS);
	}
}

// Print one of the class record lists for testing.
void printClassReqs(const ClassRecord classReqs[]) {
	for (int i = 0; i < NUM_CLASSES; i++) {
		cout << left << setw(NAME_LEN) << classReqs[i].name;
		for (int j = 0; j < NUM_STATS; j++) {
			cout << right << setw(3) << classReqs[i].statBlock[j];
		}
		cout << endl;
	}
	cout << endl;
}

// Print the two class record lists for testing.
void printAllClassReqs() {
	cout << "# Normal Class Requisites #" << endl;
	printClassReqs(CLASS_REQS);
	cout << "# Sorted Class Requisites #" << endl;
	printClassReqs(CLASS_REQS_SORTED);
	cout << endl;
}

// Roll 1d6
int rollDie() {
	return rand() % DIE_SIDES + 1;
}

// Roll 3d6 (per Method 0, et. al.)
int roll3d6() {
	int sum = 0;
	for (int i = 0; i < 3; i++) {
		sum += rollDie();	
	}
	return sum;
}

// Roll 4d6 drop lowest (per Method 1)
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

// Roll 3d6 times 6, take best (per Method 3)
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

// Make stat block per Method 0
void makeStatsMethod0 (StatBlock &stats) {
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = roll3d6();
	}
}

// Make stat block per Method 1
void makeStatsMethod1 (StatBlock &stats) {
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = roll4d6Drop1();
	}
}

// Make stat block per Method 2
void makeStatsMethod2 (StatBlock &stats) {

	// Roll 12 stats
	const int NUM_ROLLS = 12;
	int dozenStats[NUM_ROLLS];
	for (int i = 0; i < NUM_ROLLS; i++) {
		dozenStats[i] = roll3d6();
	}
	
	// Take the best 6
	sortArray(dozenStats, NUM_ROLLS);
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = dozenStats[i];	
	}
}

// Make stat block per Method 3
void makeStatsMethod3 (StatBlock &stats) {
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = roll3d6BestOf6();
	}
}

// Make one stat block per Method 4
void makeStatsMethod4 (StatBlock &stats) {
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = roll3d6();
	}
}

// List of stack-block maker functions
typedef void (*StatMakerFunc)(StatBlock&);
StatMakerFunc STAT_MAKER[NUM_METHODS] = {
	&makeStatsMethod0, &makeStatsMethod1, &makeStatsMethod2,
	&makeStatsMethod3, &makeStatsMethod4
};

// Does this method allow re-ordering the scores?
bool methodAllowsReorder (int index) {
	switch (index) {
		case 1: case 2: return true;
		default: return false;
	}
}

// Typedef for passing success tallies
typedef int PassCount[NUM_CLASSES];

// Do we qualify for this class with these stats?
bool classAllowed (ClassRecord classRec, StatBlock stats) {
	for (int i = 0; i < NUM_STATS; i++) {
		if (stats[i] < classRec.statBlock[i])
			return false;
	}
	return true;
}

// Get pass rates for generic generation method
void testMethodX (int index, PassCount passCount) {
	assert(index != 4);
	bool reorder = methodAllowsReorder(index);
	const ClassRecord *classReqs = reorder ?
		CLASS_REQS_SORTED : CLASS_REQS;
	for (int t = 0; t < NUM_TRIALS; t++) {
		StatBlock stats;
		STAT_MAKER[index](stats);
		if (reorder) {
			sortArray(stats, NUM_STATS);
		}
		for (int i = 0; i < NUM_CLASSES; i++) {
			if (classAllowed(classReqs[i], stats)) {
				passCount[i]++;
			}
		}
	}
}

// Get pass rates for Method 4
// This method needs special handling
//  because it deals with multiple full stat blocks.
void testMethod4 (PassCount passCount) {
	for (int t = 0; t < NUM_TRIALS; t++) {
		const int NUM_CHARS = 12;
		StatBlock stats[NUM_CHARS];
		for (int c = 0; c < NUM_CHARS; c++) {
			makeStatsMethod4(stats[c]);
		}
		for (int i = 0; i < NUM_CLASSES; i++) {
			for (int c = 0; c < NUM_CHARS; c++) {
				if (classAllowed(CLASS_REQS[i], stats[c])) {
					passCount[i]++;	
					break;
				}
			}
		}
	}
}

// Print test results for a generation method
void printTestResults(PassCount passCount) {
	for (int i = 0; i < NUM_CLASSES; i++) {
		double percent = (double) passCount[i] / NUM_TRIALS * 100;
		cout << left << setw(16) << CLASS_REQS[i].name;
		cout << right << setw(6) << percent << " %" << endl;		
	}
}

// Test a given stat-generation method.
void testMethod(int index) {
	cout << "# Method " << index << " #" << endl;
	PassCount passCount = {0};
	switch (index) {
		default: testMethodX(index, passCount); break;
		case 4: testMethod4(passCount); break;
	}
	printTestResults(passCount);
	cout << endl;
}

// Test each of the generation methods.
void testAllMethods() {
	for (int i = 0; i < NUM_METHODS; i++) {
		testMethod(i);	
	}
}

// Main test driver
int main(int argc, char** argv) {
	srand(time(0));
	initClassReqsSorted();
	testAllMethods();
	return 0;
}

/*
	TODO:
	- Fix output decimal places
	- Add OD&D, Bards, UA style classes?
*/
