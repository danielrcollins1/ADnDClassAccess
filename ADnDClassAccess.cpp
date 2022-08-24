/*
	Name: ADnDClassAccess
	Copyright: 2022
	Author: Daniel R. Collins
	Date: 22/08/22 23:40
	Description: 
	
		Find odds to generate any class
		from 1E AD&D defined stat-generation methods.

		Uncomment appropriate ClassRecord lists to
		switch rulesets (OD&D, AD&D, UA). 

		Assumes that 3-5 score restrictions permit
		subclasses under a specified prime class
		(unless otherwise noted).
		
		Method Roman numeral identifiers written 
		in Arabic to synch with C++, array indexing, etc.
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
const int NUM_TRIALS = 1000000;

// Typedef for a stat block
typedef int StatBlock[NUM_STATS];

// Structure for class record
struct ClassRecord {
	char name[NAME_LEN];
	StatBlock statBlock;
};

// Class records for minimum abilities
// Order: Str, Int, Wis, Dex, Con, Cha.
const ClassRecord CLASS_REQS[] = 

// OD&D and Supplements
//{
//	{"Fighter",     { 3,  3,  3,  3,  3,  3}},	
//	{"Magic-User",  { 3,  3,  3,  3,  3,  3}},	
//	{"Cleric",      { 3,  3,  3,  3,  3,  3}},	
//	{"Thief",       { 3,  3,  3,  3,  3,  3}},	
//	{"Paladin",     { 3,  3,  3,  3,  3, 17}},	
//	{"Monk",        {12,  3, 15, 15,  3,  3}},	
//	{"Assassin",    {12, 12,  3, 12,  3,  3}},	
//	{"Druid",       { 3,  3, 12,  3,  3, 14}},	
//	{"Ranger",      { 3, 12, 12,  3, 15,  3}},	
//	{"Illusionist", { 3,  3,  3, 15,  3,  3}},	
//	{"Bard",        { 9,  9,  3,  3,  3, 13}},	
//};

// AD&D 1E Players Handbook
{
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
	{"Bard",        {15, 12, 15, 15, 10, 15}},	
};

// AD&D 1E Unearthed Arcana
//{
//	{"Cavalier",    {15, 10, 10, 15, 15,  6}},	
//	{"Paladin",     {15, 10, 13, 15, 15, 17}},	
//	{"Cleric",      { 6,  6,  9,  3,  6,  6}},	
//	{"Druid",       { 6,  6, 12,  3,  6, 15}},	
//	{"Fighter",     { 9,  3,  6,  6,  7,  6}},	
//	{"Barbarian",   {15,  3,  6, 14, 15,  6}},	
//	{"Ranger",      {13, 13, 14,  6, 14,  6}},	
//	{"Magic-User",  { 3,  9,  6,  6,  6,  6}},	
//	{"Illusionist", { 3, 15,  6, 16,  3,  6}},	
//	{"Thief",       { 6,  6,  3,  9,  6,  6}},	
//	{"Acrobat",     {15,  6,  3, 16,  6,  6}},	
//	{"Assassin",    {12, 11,  3, 12,  6,  3}},	
//	{"Monk",        {15,  6, 15, 15, 11,  6}},	
//	{"Bard",        {15, 12, 15, 15, 10, 15}},	
//};

// Constant for number of classes
const int NUM_CLASSES = sizeof(CLASS_REQS) / sizeof(ClassRecord);

// Class records with minimum abilities sorted descending,
// for use in methods that allow re-ordering.
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
	int manyStats[NUM_ROLLS];
	for (int i = 0; i < NUM_ROLLS; i++) {
		manyStats[i] = roll3d6();
	}
	
	// Take the best 6
	sortArray(manyStats, NUM_ROLLS);
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = manyStats[i];	
	}
}

// Make stat block per Method 3
void makeStatsMethod3 (StatBlock &stats) {
	for (int i = 0; i < NUM_STATS; i++) {
		stats[i] = roll3d6BestOf6();
	}
}

// Make stat block per Method 4
// As Method 0, but we'll make an array of these.
void makeStatsMethod4 (StatBlock &stats) {
	makeStatsMethod0(stats);
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
//   Extra entry is to count hopeless characters
typedef int PassCount[NUM_CLASSES + 1];

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
		bool anyPassed = false;
		for (int i = 0; i < NUM_CLASSES; i++) {
			if (classAllowed(classReqs[i], stats)) {
				passCount[i]++;
				anyPassed = true;
			}
		}
		if (!anyPassed) {
			passCount[NUM_CLASSES]++;			
		}
	}
}

// Get pass rates for Method 4
// This generation method needs special handling
//  because it deals with an array of full stat blocks.
void testMethod4 (PassCount passCount) {
	for (int t = 0; t < NUM_TRIALS; t++) {
		const int NUM_CHARS = 12;
		StatBlock stats[NUM_CHARS];
		for (int c = 0; c < NUM_CHARS; c++) {
			makeStatsMethod4(stats[c]);
		}
		bool anyPassed = false;
		for (int i = 0; i < NUM_CLASSES; i++) {
			for (int c = 0; c < NUM_CHARS; c++) {
				if (classAllowed(CLASS_REQS[i], stats[c])) {
					passCount[i]++;	
					anyPassed = true;
					break;
				}
			}
		}
		if (!anyPassed) {
			passCount[NUM_CLASSES]++;			
		}
	}
}

// Get pass counts for any generation method.
void getMethodPassCounts (int index, PassCount passCount) {
	switch (index) {
		default: testMethodX(index, passCount); break;
		case 4: testMethod4(passCount); break;
	}
}

// Compute the passing percentage from a given pass count.
double getPassPercent(int passCount) {
	return (double) passCount / NUM_TRIALS * 100;	
}

// Print one line of testing results
void printTestLine (const char* name, int passCount) {
	cout << left << setw(NAME_LEN) << name
		<< right << setw(6) << getPassPercent(passCount) 
		<< " %" << endl;
}

// Print test results for a generation method
void printTestResults(PassCount passCount) {
	cout << fixed << showpoint << setprecision(2);
	for (int i = 0; i < NUM_CLASSES; i++) {
		printTestLine(CLASS_REQS[i].name, passCount[i]);
	}
	printTestLine("NO CLASS", passCount[NUM_CLASSES]);
}

// Test a given stat-generation method.
void testMethod(int index) {
	cout << "# Method " << index << " #" << endl;
	PassCount passCount = {0};
	getMethodPassCounts(index, passCount);
	printTestResults(passCount);
	cout << endl;
}

// Test each of the generation methods.
void testAllMethods() {
	for (int i = 0; i < NUM_METHODS; i++) {
		testMethod(i);	
	}
}

// Make master table of results, copyable to spreadsheet
void makeMasterTable () {

	// Generate pass counts
	PassCount passCount[NUM_METHODS] = {0};
	for (int m = 0; m < NUM_METHODS; m++) {
		getMethodPassCounts(m, passCount[m]);
	}

	// Print the table
	cout << "# Master Access Table #" << endl;
	cout << fixed << setprecision(0);
	for (int c = 0; c < NUM_CLASSES; c++) {
		cout << CLASS_REQS[c].name << "\t";
		for (int m = 0; m < NUM_METHODS; m++) {
			cout << getPassPercent(passCount[m][c]) << "\t";
		}
		cout << endl;		
	}
	
	// Print no-class values
	cout << "NO CLASS" << "\t";
	for (int m = 0; m < NUM_METHODS; m++) {
		cout << getPassPercent(passCount[m][NUM_CLASSES]) << "\t";
	}
	cout << endl;
}

// Main test driver
int main(int argc, char** argv) {
	srand(time(0));
	initClassReqsSorted();
	testAllMethods();
	//makeMasterTable();
	return 0;
}
