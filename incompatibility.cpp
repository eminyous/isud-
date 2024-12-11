#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

// Reading the solution from the text file
unordered_map<string, double> readSolution(const string& logFileName) {
    unordered_map<string, double> solution;
    ifstream logFile(logFileName);
    if (!logFile.is_open()) {
        cerr << "Error: Could not open log file " << logFileName << endl;
        return solution;
    }

    string line;
    bool firstLine = true;
    while (getline(logFile, line)) {
        // Skipping objective value
        if (firstLine) {
            firstLine = false;
            continue;
        }

        stringstream ss(line);
        string varName;
        double value;
        string obj;

        if (ss >> varName >> value >> obj) {
            solution[varName] = value;
        }
    }
    logFile.close();
    return solution;
}

// Parsing a column's elements from its variable name
vector<int> parseColumnElements(const string& colName) {
    vector<int> elements;
    stringstream ss(colName);
    string token;
    while (getline(ss, token, '_')) {
        if (isdigit(token[0])) {
            elements.push_back(stoi(token));
        }
    }
    return elements;
}

// Calculating the incompatibility degree of a column A_j with respect to the working basis P
int calculateIncompatibilityDegree(const string& colJ, const unordered_set<string>& P,
    const unordered_map<string, vector<int>>& rowsCovered) {
    int incompatibilityDegree = 0;

    for (const string& colL : P) {
        const vector<int>& rowsL = rowsCovered.at(colL);
        const vector<int>& rowsJ = rowsCovered.at(colJ);

        if (rowsL.size() < 2) continue; // Avoid accessing invalid indices

        for (size_t t = 0; t < rowsL.size() - 1; ++t) {
            bool covers_jt = find(rowsJ.begin(), rowsJ.end(), rowsL[t]) != rowsJ.end();
            bool covers_jt1 = find(rowsJ.begin(), rowsJ.end(), rowsL[t + 1]) != rowsJ.end();

            if ((covers_jt && !covers_jt1) || (!covers_jt && covers_jt1)) {
                incompatibilityDegree += 1;
            }
            else if (covers_jt && covers_jt1 && rowsL[t + 1] - rowsL[t] > 1) {
                incompatibilityDegree += 2;
            }
        }
    }

    return incompatibilityDegree;
}

// Working basis P from the solution
unordered_set<string> findWorkingBasis(const unordered_map<string, double>& solution) {
    unordered_set<string> workingBasis;

    for (const auto& it : solution) {
        if (it.second == 1.0) {
            workingBasis.insert(it.first);
        }
    }

    return workingBasis;
}

// Finding compatible columns for each column based on the incompatibility degree
unordered_map<string, vector<string>> findCompatibleColumns(const unordered_map<string, double>& solution,
    const unordered_set<string>& workingBasis,
    const unordered_map<string, vector<int>>& rowsCovered) {
    unordered_map<string, vector<string>> compatibilityMap;

    for (auto it1 = solution.begin(); it1 != solution.end(); ++it1) {
        if (it1->second != 1.0) continue;  // not part of the solution

        int incompatibilityDegree = calculateIncompatibilityDegree(it1->first, workingBasis, rowsCovered);
        if (incompatibilityDegree == 0) {
            for (auto it2 = solution.begin(); it2 != solution.end(); ++it2) {
                if (it1 == it2) continue;  // variable with itself
                if (it2->second != 1.0) continue;  // not part of the solution

                int incompatibilityDegree2 = calculateIncompatibilityDegree(it2->first, workingBasis, rowsCovered);
                if (incompatibilityDegree2 == 0) {
                    compatibilityMap[it1->first].push_back(it2->first);
                }
            }
        }
    }
    return compatibilityMap;
}

void outputResults(const unordered_map<string, vector<string>>& compatibilityMap, const string& instanceName) {
    string fileName = "compat_" + instanceName + ".txt";
    ofstream outFile(fileName);
    if (!outFile.is_open()) {
        cerr << "Error: Could not open output file " << fileName << endl;
        return;
    }

    for (const auto& entry : compatibilityMap) {
        outFile << entry.first << " :";
        for (const auto& compatibleVar : entry.second) {
            outFile << " " << compatibleVar;
        }
        outFile << endl;
    }
    outFile.close();
}

int main() {
    // Log file from SCIP containing the feasible solution
    string logFileName = "log_AS65-2.txt";

    unordered_map<string, double> solution = readSolution(logFileName);

    // cover
    unordered_map<string, vector<int>> rowsCovered;
    for (const auto& entry : solution) {
        rowsCovered[entry.first] = parseColumnElements(entry.first);
    }

    //working basis
    unordered_set<string> workingBasis = findWorkingBasis(solution);

    //compatible columns
    unordered_map<string, vector<string>> compatibilityMap = findCompatibleColumns(solution, workingBasis, rowsCovered);

    string instanceName = "AS65-2";
    outputResults(compatibilityMap, instanceName);

    return 0;
}
