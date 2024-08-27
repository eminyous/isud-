#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include "scip/cons_linear.h"
#include <vector>
#include <iostream>
#include <random>
#include <cmath>

// Gomory cuts
void addGomoryCuts(SCIP* scip) {
    int nRows;
    SCIP_ROW** lpRows;
    SCIP_CALL_ABORT(SCIPgetLPRowsData(scip, &lpRows, &nRows));

    for (int i = 0; i < nRows; ++i) {
        SCIP_ROW* row = lpRows[i];
        if (SCIProwGetDualsol(row) < 0.0) { // Check row- candidate for Gomory cuts
            SCIP_Real* vals = SCIProwGetVals(row);
            int nNonz = SCIProwGetNNonz(row);
            SCIP_COL** cols = SCIProwGetCols(row); // Correct way to get columns

            // new Gomory cut row
            SCIP_ROW* gomoryRow;
            SCIP_CALL_ABORT(SCIPcreateEmptyRowCons(scip, &gomoryRow, NULL, "gomory_cut", -SCIPinfinity(scip), 0.0, FALSE, FALSE, TRUE));
            SCIP_CALL_ABORT(SCIPcacheRowExtensions(scip, gomoryRow));

            for (int j = 0; j < nNonz; ++j) {
                double fractionalPart = vals[j] - floor(vals[j]);
                if (fractionalPart > 0.0) {
                    SCIP_VAR* var = SCIPcolGetVar(cols[j]); // Get the variable associated with the column
                    SCIP_CALL_ABORT(SCIPaddVarToRow(scip, gomoryRow, var, fractionalPart));
                }
            }

            SCIP_Bool infeasible = FALSE; // Declare the infeasibility variable
            // Add Gomory cut
            SCIP_CALL_ABORT(SCIPaddCut(scip, NULL, gomoryRow, FALSE, &infeasible));

            // Release the created row
            SCIP_CALL_ABORT(SCIPreleaseRow(scip, &gomoryRow));
        }
    }
}

// Random projections- Gaussian distribution
std::vector<double> randomProjection(const std::vector<double>& vector, int targetDim) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dis(0, 1);

    std::vector<double> projection(targetDim);
    for (int i = 0; i < targetDim; ++i) {
        projection[i] = 0;
        for (double val : vector) {
            projection[i] += val * dis(gen);
        }
    }
    return projection;
}


int main(int argc, char** argv) {
    // Command line argument input file
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <mps-file>" << std::endl;
        return 1;
    }

    const char* mpsFile = argv[1];

    // Initialize SCIP
    SCIP* scip = NULL;
    SCIP_CALL(SCIPcreate(&scip));
    SCIP_CALL(SCIPincludeDefaultPlugins(scip));
    SCIP_CALL(SCIPcreateProbBasic(scip, "ILP_Problem"));

    //problem
    SCIP_CALL(SCIPreadProb(scip, mpsFile, NULL));

    // initial LP relaxation
    SCIP_CALL(SCIPsolve(scip));

    // Add Gomory cuts
    addGomoryCuts(scip);

    // Re-solve 
    SCIP_CALL(SCIPsolve(scip));

    // Optimal solution
    SCIP_SOL* sol = SCIPgetBestSol(scip);
    if (sol != NULL) {
        std::cout << "Optimal solution found: " << SCIPgetSolOrigObj(scip, sol) << std::endl;
    }
    else {
        std::cout << "No solution found." << std::endl;
    }

    // Free SCIP
    SCIP_CALL(SCIPfree(&scip));
    return 0;
}
