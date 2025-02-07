
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <filesystem>
using namespace std;

#define REPLACE 0
#define DELETE 1
#define INSERT 2

/* 
Writted by Joshua Jones, 2025-01-13

This is compatible with C++17!

Made some serious code optimizations, managed to get the 100k runtime from 11min to ~2.5min! the 30k runs in 18sec now to!
*/

// Using this as an easier method for interfacing w/ files. Pulls all the file data into std::vector. Means I can spoof randomly accessing specific lines, and it's less messy.
// Note: With absolutely rediculously sized text files this will likely cause a crash. Frankly however if ur running this against a text file that crashs notepad I think u have other problems. 
std::vector<std::string> GetFile(const std::string &thePath) {

    std::vector<std::string> aDataSet;

    std::ifstream aFileStream(thePath);
    if (aFileStream.is_open()) {
        int anIndex = 0;

        std::string aCollector;

        while (getline(aFileStream, aCollector)) {
            aDataSet.push_back(aCollector);
            anIndex = anIndex + 1;
        }

        aFileStream.close();

        return aDataSet;
    }

    return std::vector<std::string>(1, "");
}

// Dynamically fills out an array and deletes itself as it goes, storing the diagonal in a variable for use in solving the instruction set.
// Note: This function is periodically recalled in the event that the number of diagonal columns is to low. `theOldFileIndexToStopAt` is changed upon recall so it does not regenerate the whole diagonal array, just what it needs.
// Made some serious code optimizations here, managed to get the 100k runtime from 11min to ~2.5min! the 30k runs in 18sec now to!
std::vector<std::vector<int>> GenerateDiagonalArray(std::vector<std::string> &theOldFile, std::vector<std::string> &theNewFile, const int &theNumberOfColumns, const int &theOldFileIndexToStopAt, const std::vector<std::vector<int>> &theOldDiagonalArray = std::vector<std::vector<int>>()) {

    std::vector<std::vector<int>> aWorkingArray;
    std::vector<std::vector<int>> aDiagonalArray;

    const int anOldFileLineCount = (int)theOldFile.size();
    const int aNewFileLineCount = (int)theNewFile.size();

    aWorkingArray.resize(2, std::vector<int>(anOldFileLineCount + 1, 0));
    aDiagonalArray.resize(theNumberOfColumns, std::vector<int>(2, 0));

    int anIndex = 0;
    while (anIndex < (int)aWorkingArray[0].size()) {
        aWorkingArray[0][anIndex] = anIndex;
        anIndex = anIndex + 1;
    }

    aWorkingArray[1][0] = 1;

    int anOldFileIndex = anOldFileLineCount;
    int aNewFileIndex = 0;
    bool aWorkingIndex = false;

    int aDiagonalIndex;
    
    const int aMinimumDiagonalBound = (((1 - theNumberOfColumns) / 2) - 1);
    const int aMaximumDiagonalBound = (((theNumberOfColumns - 1) / 2) + 1);
    const int aDiagonalArrayIndexConstant = ((theNumberOfColumns - 1) / 2);

    int aDiagonalLineLength = 1;
    anIndex = 0;

    while (anOldFileIndex > 0) {
        aDiagonalIndex = anOldFileIndex - aNewFileIndex;
        if ((aMinimumDiagonalBound < aDiagonalIndex) && (aDiagonalIndex < aMaximumDiagonalBound)) {
            aDiagonalArray[aDiagonalIndex + aDiagonalArrayIndexConstant].resize(aDiagonalLineLength, 0);
        }

        if (anIndex < std::min({aNewFileLineCount, anOldFileLineCount})) {
            aDiagonalLineLength = aDiagonalLineLength + 1;
        }
        anIndex = anIndex + 1;
        anOldFileIndex = anOldFileIndex - 1;
    }

    while (aNewFileIndex <= aNewFileLineCount) {
        if (anIndex > std::max({aNewFileLineCount, anOldFileLineCount})) {
            aDiagonalLineLength = aDiagonalLineLength - 1;
        }

        aDiagonalIndex = anOldFileIndex - aNewFileIndex;
        if ((aMinimumDiagonalBound < aDiagonalIndex) && (aDiagonalIndex < aMaximumDiagonalBound)) {
            aDiagonalArray[aDiagonalIndex + aDiagonalArrayIndexConstant].resize(aDiagonalLineLength, 0);
        }

        anIndex = anIndex + 1;
        aNewFileIndex = aNewFileIndex + 1;
    }

    std::vector<int> aSetOfDiagonalIndexs(theNumberOfColumns, 1);

    if ((theOldDiagonalArray.size() > 0) && (anOldFileLineCount < aNewFileLineCount)) {
        anIndex = aDiagonalArray.size() - theOldDiagonalArray.size() - 1;
        while ((anIndex - (aDiagonalArray.size() - theOldDiagonalArray.size() - 1)) < theOldDiagonalArray.size()) {
            aDiagonalArray[anIndex] = theOldDiagonalArray[anIndex - (aDiagonalArray.size() - theOldDiagonalArray.size() - 1)];
            aSetOfDiagonalIndexs[anIndex] = aDiagonalArray[anIndex].size() + 1;
            anIndex = anIndex + 1;
        }
    }

    int anOldFileIndexMinusOne;
    int aZerodDiagonalIndex;

    int aWorkingBottomLeftSpot;
    int aWorkingTopLeftSpot;
    int aWorkingTopSpot;

    anOldFileIndex = 1;
    aNewFileIndex = 1;

    const int anOldFileLineCountPlusOne = anOldFileLineCount + 1;
    const int aNewFileLineCountPlusOne = anOldFileLineCount + 1;
    
    int aMinimumFindingWorker;
    int aNewFileIndexMinusOne;

    while (aNewFileIndex < aNewFileLineCountPlusOne) {

        anOldFileIndex = aNewFileIndex - aDiagonalArrayIndexConstant;
        if (anOldFileIndex < 1) {
            anOldFileIndex = 1;
        }

        aNewFileIndexMinusOne = aNewFileIndex - 1;

        while ((anOldFileIndex < anOldFileLineCountPlusOne) && (anOldFileIndex < theOldFileIndexToStopAt)) {
            
            aDiagonalIndex = anOldFileIndex - aNewFileIndex;
            aZerodDiagonalIndex = aDiagonalIndex + aDiagonalArrayIndexConstant;

            if (((aMinimumDiagonalBound) < (aDiagonalIndex)) && ((aDiagonalIndex) < (aMaximumDiagonalBound))) {
                
                // Made sure that I try not to do math twice in here if I don't need to, hence all the random variables. Also had to remove any external function calls, so std::min had to go. (which is to bad, I liked that method lol)
                if (aSetOfDiagonalIndexs[aZerodDiagonalIndex] < (int)aDiagonalArray[aZerodDiagonalIndex].size()) {
                    anOldFileIndexMinusOne = anOldFileIndex - 1;
                    aWorkingBottomLeftSpot = aWorkingArray[!aWorkingIndex][anOldFileIndexMinusOne];
                    aWorkingTopLeftSpot = aWorkingArray[aWorkingIndex][anOldFileIndexMinusOne];
                    aWorkingTopSpot = aWorkingArray[aWorkingIndex][anOldFileIndex];

                    aMinimumFindingWorker = (aWorkingTopLeftSpot < aWorkingBottomLeftSpot) ? aWorkingTopLeftSpot : aWorkingBottomLeftSpot; // From my experimentation, this is the fasted method of getting the minimum of 3 numbers I could find.
                    aWorkingArray[!aWorkingIndex][anOldFileIndex] = ((aMinimumFindingWorker < aWorkingTopSpot) ? aMinimumFindingWorker : aWorkingTopSpot) + (theOldFile[anOldFileIndexMinusOne] != theNewFile[aNewFileIndexMinusOne]);

                    aDiagonalArray[aZerodDiagonalIndex][aSetOfDiagonalIndexs[aZerodDiagonalIndex]] = aWorkingArray[!aWorkingIndex][anOldFileIndex];
                    aSetOfDiagonalIndexs[aZerodDiagonalIndex] = aSetOfDiagonalIndexs[aZerodDiagonalIndex] + 1;
                } else {
                    aWorkingArray[!aWorkingIndex][anOldFileIndex] = aDiagonalArray[aZerodDiagonalIndex][aSetOfDiagonalIndexs[aZerodDiagonalIndex]];
                }
            } else {
                
                anOldFileIndexMinusOne = anOldFileIndex - 1;
                aWorkingBottomLeftSpot = aWorkingArray[!aWorkingIndex][anOldFileIndexMinusOne];
                aWorkingTopLeftSpot = aWorkingArray[aWorkingIndex][anOldFileIndexMinusOne];
                aWorkingTopSpot = aWorkingArray[aWorkingIndex][anOldFileIndex];

                aMinimumFindingWorker = (aWorkingTopLeftSpot < aWorkingBottomLeftSpot) ? aWorkingTopLeftSpot : aWorkingBottomLeftSpot;
                aWorkingArray[!aWorkingIndex][anOldFileIndex] = ((aMinimumFindingWorker < aWorkingTopSpot) ? aMinimumFindingWorker : aWorkingTopSpot) + (theOldFile[anOldFileIndexMinusOne] != theNewFile[aNewFileIndexMinusOne]);

            }

            anOldFileIndex = anOldFileIndex + 1;
        }

        aNewFileIndex = aNewFileIndex + 1;

        aWorkingIndex = !aWorkingIndex;
        aWorkingArray[!aWorkingIndex][0] = aNewFileIndex;
    }

    anIndex = 0;
    while (anIndex < (int)aDiagonalArray.size()) {
        aDiagonalArray[anIndex][0] = abs(((theNumberOfColumns - 1) / 2) - anIndex);
        anIndex = anIndex + 1;
    }

    return aDiagonalArray;
}

// This basically translates the standard XY coords into the diagonal coords and returns a pointer to the correct location.
int *GetSpot(std::vector<std::vector<int>> &theArray, const int &theY, const int &theX) {

    int aDiagonalIndex = abs((int)((theX - theY) + ((theArray.size() - 1) / 2)));
    int aColumnIndex;

    if (aDiagonalIndex > (int)theArray.size()) {
        return nullptr;
    }

    if (aDiagonalIndex < (((int)theArray.size() - 1) / 2)) {
        aColumnIndex = theX;
    } else {
        aColumnIndex = theY;
    }

    return &(theArray[aDiagonalIndex][aColumnIndex]);
}

// Using this rather than storing the whole instruction string for memmory saving purposes.
struct Instruction {
    unsigned char itsInstruction : 2;
    int itsOldFileIndex : 24;
    int itsNewFileIndex : 24;
    
    Instruction(const unsigned char &theInstruction, const int &theOldFileIndex = 0, const int &theNewFileIndex = 0) {
    	itsInstruction = theInstruction;
        itsOldFileIndex = theOldFileIndex;
        itsNewFileIndex = theNewFileIndex;
    }

    Instruction() {}
};

// This takes the now fully filled array and backtracks, storing the required steps in a vector as it goes. (It stores rather than prints so the vector can be reversed after the fact)
// You may notice this no longer stores a string and now stores a struct called `Instruction`. This is for the purpose of saving on memmory.
std::vector<Instruction> SolveSteps(std::vector<std::vector<int>> &theArray, std::vector<std::string> &theOldFile, std::vector<std::string> &theNewFile, int &theDiagonalSize) {
    int anOldFileIndex = theOldFile.size();
    int aNewFileIndex = theNewFile.size();

    std::vector<Instruction> aSetOfInstructions;

    while ((anOldFileIndex > 0) || (aNewFileIndex > 0)) {
        if (theOldFile[anOldFileIndex - 1] != theNewFile[aNewFileIndex - 1]) {

            while (!GetSpot(theArray, aNewFileIndex, anOldFileIndex - 2) || !GetSpot(theArray, aNewFileIndex - 2, anOldFileIndex)) {
                std::cout << "derp2";
                theDiagonalSize = theDiagonalSize + std::pow((std::pow(std::min({theOldFile.size(), theNewFile.size()}) / 30000, 2) + 2) - 1, 2) + 1;
                std::cout << "\nWARNING: High amount of drift from diagonal detected. (Current OldFileIndex is at ";
                std::cout << anOldFileIndex;
                std::cout << ")";
                std::cout << "\nResizing diagonal to ";
                std::cout << theDiagonalSize;
                std::cout << " columns...\n";
                theArray = GenerateDiagonalArray(theOldFile, theNewFile, theDiagonalSize, anOldFileIndex + 1, theArray);
            }

            if ((aNewFileIndex > 0) && (anOldFileIndex > 0) && (*GetSpot(theArray, aNewFileIndex - 1, anOldFileIndex - 1) == (*GetSpot(theArray, aNewFileIndex, anOldFileIndex) - 1))) {

                aSetOfInstructions.push_back(Instruction(REPLACE, anOldFileIndex, aNewFileIndex));
                
                anOldFileIndex = anOldFileIndex - 1;
                aNewFileIndex = aNewFileIndex - 1;
            } else if ((anOldFileIndex > 0) && (*GetSpot(theArray, aNewFileIndex, anOldFileIndex - 1) == (*GetSpot(theArray, aNewFileIndex, anOldFileIndex) - 1))) {
                
                aSetOfInstructions.push_back(Instruction(DELETE, anOldFileIndex, aNewFileIndex));

                anOldFileIndex = anOldFileIndex - 1;
            } else {
                
                aSetOfInstructions.push_back(Instruction(INSERT, anOldFileIndex, aNewFileIndex));

                aNewFileIndex = aNewFileIndex - 1;
            }

        } else {
            anOldFileIndex = anOldFileIndex - 1;
            aNewFileIndex = aNewFileIndex - 1;
        }
    }

    std::reverse(aSetOfInstructions.begin(), aSetOfInstructions.end());

    return aSetOfInstructions;
}

// Literally just and prints the instructions
void DisplayInstructions(std::vector<Instruction> &theInstructionSet, std::vector<std::string> &theOldFile, std::vector<std::string> &theNewFile) {
    for (auto anInstruction : theInstructionSet) {
        switch (anInstruction.itsInstruction) {
            case REPLACE:
                std::cout << "replace " + std::to_string(anInstruction.itsOldFileIndex) + " {" + theOldFile[anInstruction.itsOldFileIndex - 1] + "} -> {" + theNewFile[anInstruction.itsNewFileIndex - 1] + "}\n";
                break;
            
            case DELETE:
                std::cout << "delete  " + std::to_string(anInstruction.itsOldFileIndex) + " {" + theOldFile[anInstruction.itsOldFileIndex - 1] + "}\n";
                break;

            case INSERT:
                std::cout << "insert  " + std::to_string(anInstruction.itsOldFileIndex) + " {" + theNewFile[anInstruction.itsNewFileIndex - 1] + "}\n";
                break;
        }
    }
}

int main(int argc, char ** argv) {

    if (argc < 3) {
        std::cout << "Invalid number of arguments.\n";
        std::cout << "Command line: `editdistance <old_file_path> <new_file_path>`";
        return 1;
    }

    std::string theOldFilePath = argv[1];
    std::string theNewFilePath = argv[2];

    std::vector<std::string> anOldFile;
    std::vector<std::string> aNewFile;

    // Check if the old file exists
    if (std::filesystem::exists(theOldFilePath)) {
        anOldFile = GetFile(theOldFilePath);
    } else {
        std::cout << "Old file '" + theOldFilePath + "' does not exist.";
        return 2;
    }

    // Check if the new file exists
    if (std::filesystem::exists(theNewFilePath)) {
        aNewFile = GetFile(theNewFilePath);
    } else {
        std::cout << "New file '" + theNewFilePath + "' does not exist.";
        return 2;
    }
    std::vector<std::vector<int>> aPreSolvedArray;
    std::vector<Instruction> aSetOfInstructions;

    // approximates the max needed diagonal size. `SolveSteps` will recall `GenerateDiagonalArray` with an increased size if it's not enough to get the data it needs.
    int aDiagonalSize = ((anOldFile.size() > aNewFile.size()) ? anOldFile.size() : aNewFile.size()) / 2;

    if (aDiagonalSize > 1501) {
        aDiagonalSize = 1501; // magic numbers. Prevents a massacre of the memmory and its family/friends 
    } else if ((aDiagonalSize % 2) == 0) {
        aDiagonalSize = aDiagonalSize + 1;
    }

    // Generate the array and solve for steps
    aPreSolvedArray = GenerateDiagonalArray(anOldFile, aNewFile, aDiagonalSize, anOldFile.size() + 1);
    aSetOfInstructions = SolveSteps(aPreSolvedArray, anOldFile, aNewFile, aDiagonalSize);

    // Display the list of instructions
    DisplayInstructions(aSetOfInstructions, anOldFile, aNewFile);

    return 0;
}