#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>

// Read problem matrix
std::unordered_map<std::string, std::pair<double, double>> parseProblemMatrix(const std::string& filename) {
    std::unordered_map<std::string, std::pair<double, double>> problemMatrix;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return problemMatrix;
    }

    std::string line;
    std::getline(file, line); // Skip the first line

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string variable;
        double value, obj;

        // Format: "variable_name value (obj:objective_value)"
        iss >> variable >> value;
        std::string objString;
        if (iss >> objString) {
            size_t pos1 = objString.find(":");
            size_t pos2 = objString.find(")");
            if (pos1 != std::string::npos && pos2 != std::string::npos) {
                objString = objString.substr(pos1 + 1, pos2 - pos1 - 1);
                try {
                    obj = std::stod(objString);
                    problemMatrix[variable] = std::make_pair(value, obj);
                }
                catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid objective value for variable " << variable << std::endl;
                }
            }
            else {
                std::cerr << "Failed to parse objective value for " << variable << std::endl;
            }
        }
    }

    file.close();
    return problemMatrix;
}

// Parse the compatible columns
std::unordered_set<std::string> parseCompatibleColumns(const std::string& filename) {
    std::unordered_set<std::string> compatibleColumns;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return compatibleColumns;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string temp, column;
        iss >> temp; // Skip the first word, which is not relevant

        while (iss >> column) {
            compatibleColumns.insert(column);
        }
    }

    file.close();
    return compatibleColumns;
}

// Classify columns and extract coefficients
void classifyCols(
    const std::unordered_map<std::string, std::pair<double, double>>& problemMatrix,
    const std::unordered_set<std::string>& compatibleColumns,
    std::unordered_map<std::string, std::pair<double, double>>& coefficients_P,
    std::unordered_map<std::string, std::pair<double, double>>& coefficients_I) {

    for (const auto& [variable, value_obj] : problemMatrix) {
        // Only consider positive-valued variables for compatibility
        if (value_obj.first > 0.0 && compatibleColumns.find(variable) != compatibleColumns.end()) {
            coefficients_P[variable] = value_obj; // Compatible
        }
        else {
            coefficients_I[variable] = value_obj; // Incompatible
        }
    }
}

int main() {
    std::string initialSolutionFile = "log_AS65-2.txt"; // Problem Matrix
    std::string compatibleColumnsFile = "compat_AS65-2.txt";

    auto problemMatrix = parseProblemMatrix(initialSolutionFile);

    auto compatibleColumns = parseCompatibleColumns(compatibleColumnsFile);

    std::unordered_map<std::string, std::pair<double, double>> coefficients_P;
    std::unordered_map<std::string, std::pair<double, double>> coefficients_I;
    classifyCols(problemMatrix, compatibleColumns, coefficients_P, coefficients_I);

    // Write compatible coefficients to a file
    std::ofstream compatibleFile("compatible_coefficients-AS65-2.txt");
    if (compatibleFile.is_open()) {
        for (const auto& [var, val_obj] : coefficients_P) {
            compatibleFile << var << " : " << val_obj.first << " (obj: " << val_obj.second << ")" << std::endl;
        }
        compatibleFile.close();
    }
    else {
        std::cerr << "Error opening file for writing compatible coefficients." << std::endl;
    }

    // Write incompatible coefficients to a file
    std::ofstream incompatibleFile("incompatible_coefficients-AS65-2.txt");
    if (incompatibleFile.is_open()) {
        for (const auto& [var, val_obj] : coefficients_I) {
            incompatibleFile << var << " : " << val_obj.first << " (obj: " << val_obj.second << ")" << std::endl;
        }
        incompatibleFile.close();
    }
    else {
        std::cerr << "Error opening file for writing incompatible coefficients." << std::endl;
    }

    // Verify the coefficients
    std::cout << "Compatible Coefficients (P) written to compatible_coefficients-AS65-2.txt" << std::endl;
    std::cout << "Incompatible Coefficients (I) written to incompatible_coefficients-AS65-2.txt" << std::endl;

    return 0;
}
