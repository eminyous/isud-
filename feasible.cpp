#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

// Extracting the base file name from the full file path (optional)
string extractFileName(const string& filePath) {
    size_t pos = filePath.find_last_of("\\/");
    if (pos != string::npos) {
        return filePath.substr(pos + 1);
    }
    return filePath;
}

// Removing the file extension from the file name (optional)
string removeFileExtension(const string& fileName) {
    size_t pos = fileName.find_last_of('.');
    if (pos != string::npos) {
        return fileName.substr(0, pos);
    }
    return fileName;
}

// Visiting directories and gathering MPS file paths (optional)
vector<string> gatherMpsFiles(const string& rootDir) {
    vector<string> mpsFiles;
    for (const auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (entry.path().extension() == ".mps") {
            mpsFiles.push_back(entry.path().string());
        }
    }
    return mpsFiles;
}


//Solving to find initial feasible solution
void solveWithScip(const string& mpsFile) {
    SCIP* scip = nullptr;
    SCIP_RETCODE retcode;

    // Create SCIP environment
    retcode = SCIPcreate(&scip);
    if (retcode != SCIP_OKAY) {
        cerr << "Error: Failed to create SCIP instance." << endl;
        return;
    }

    // Include default SCIP plugins
    retcode = SCIPincludeDefaultPlugins(scip);
    if (retcode != SCIP_OKAY) {
        cerr << "Error: Failed to include default plugins." << endl;
        SCIPfree(&scip);
        return;
    }

    try {
        // Create a problem instance in SCIP
        retcode = SCIPcreateProbBasic(scip, "problem");
        if (retcode != SCIP_OKAY) {
            cerr << "Error: Failed to create problem instance." << endl;
            SCIPfree(&scip);
            return;
        }

        // Read the MPS file with SCIP
        retcode = SCIPreadProb(scip, mpsFile.c_str(), nullptr);
        if (retcode != SCIP_OKAY) {
            cerr << "Error: Failed to read problem from file." << endl;
            SCIPfree(&scip);
            return;
        }

        // Set SCIP parameters
        retcode = SCIPsetIntParam(scip, "limits/solutions", 1);  // Stop after the first feasible solution
        if (retcode != SCIP_OKAY) {
            cerr << "Error: Failed to set solutions limit parameter." << endl;
            SCIPfree(&scip);
            return;
        }
        retcode = SCIPsetRealParam(scip, "limits/time", 60.0);  // Set a time limit (optional)
        if (retcode != SCIP_OKAY) {
            cerr << "Error: Failed to set time limit parameter." << endl;
            SCIPfree(&scip);
            return;
        }

        // Solve the problem with SCIP
        retcode = SCIPsolve(scip);
        if (retcode != SCIP_OKAY) {
            cerr << "Error: Failed to solve the problem." << endl;
            SCIPfree(&scip);
            return;
        }

        // Get the best solution found
        SCIP_SOL* sol = SCIPgetBestSol(scip);
        if (sol == nullptr) {
            cerr << "No feasible solution found for " << mpsFile << endl;
            SCIPfree(&scip);
            return;
        }

        // Generate log file name based on the MPS file name
        string baseFileName = extractFileName(mpsFile);
        string logFileName = "log_" + removeFileExtension(baseFileName) + ".txt";

        // Write the initial feasible solution to the log file
        ofstream logFile(logFileName);
        if (logFile.is_open()) {
            // Objective value
            SCIP_Real objValue = SCIPgetSolOrigObj(scip, sol);
            logFile << "objective value: " << objValue << endl;

            // Variables and their values
            int nvars = SCIPgetNVars(scip);
            SCIP_VAR** vars = SCIPgetVars(scip);

            for (int i = 0; i < nvars; ++i) {
                SCIP_Real val = SCIPgetSolVal(scip, sol, vars[i]);
                SCIP_Real objCoef = SCIPvarGetObj(vars[i]);
                logFile << SCIPvarGetName(vars[i]) << "\t" << val << "\t" << "(obj:" << objCoef << ")" << endl;
            }
            logFile.close();
        }

        // Free problem instance in SCIP
        retcode = SCIPfreeProb(scip);
        if (retcode != SCIP_OKAY) {
            cerr << "Error: Failed to free problem instance." << endl;
        }
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    // Free SCIP environment
    retcode = SCIPfree(&scip);
    if (retcode != SCIP_OKAY) {
        cerr << "Error: Failed to free SCIP instance." << endl;
    }
}

int main() {
    // Root directory containing all the test folders
    string rootDir = //path;

    // Gather all MPS file paths
    vector<string> mpsFiles = gatherMpsFiles(rootDir);

    for (const string& mpsFile : mpsFiles) {
        cout << "Solving " << mpsFile << " with SCIP..." << endl;
        solveWithScip(mpsFile);
    }

    return 0;
}
