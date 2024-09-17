#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <iomanip>
#include <cmath>

using namespace std;

void compareFiles(const string& file1, const string& file2) {
    ifstream inputFile1(file1);
    ifstream inputFile2(file2);

    if (!inputFile1.is_open() || !inputFile2.is_open()) {
        cerr << "Error opening files." << endl;
        return;
    }

    map<string, double> loadMap1, loadMap2;

    string line;
    while (getline(inputFile1, line)) {
        istringstream iss(line);
        string gateName;
        double load;
        iss >> gateName >> load;
        loadMap1[gateName] = load;
    }

    while (getline(inputFile2, line)) {
        istringstream iss(line);
        string gateName;
        double load;
        iss >> gateName >> load;
        loadMap2[gateName] = load;
    }

    inputFile1.close();
    inputFile2.close();

    bool filesAreEquivalent = true;

    // Compare loadMap1 with loadMap2
    for (const auto& pair : loadMap1) {
        const string& gateName = pair.first;
        double load1 = pair.second;

        if (loadMap2.find(gateName) == loadMap2.end()) {
            cout << "Gate " << gateName << " is missing in " << file2 << endl;
            filesAreEquivalent = false;
        } else {
            double load2 = loadMap2[gateName];
            int tmp = load1*1e6;
            int tmp1=load2*1e6;
            if (abs(tmp - tmp1) > 1) {
                cout << "Difference in gate " << gateName << ": "
                     << file1 << " has load " << load1 << ", "
                     << file2 << " has load " << load2 << endl;
                filesAreEquivalent = false;
            }
        }
    }

    // Check for gates that are in loadMap2 but not in loadMap1
    for (const auto& pair : loadMap2) {
        const string& gateName = pair.first;
        if (loadMap1.find(gateName) == loadMap1.end()) {
            cout << "Gate " << gateName << " is missing in " << file1 << endl;
            filesAreEquivalent = false;
        }
    }

    if (filesAreEquivalent) {
        cout << "The files are equivalent." << endl;
    } else {
        cout << "The files are different." << endl;
    }
}

int main(int argc,char**argv) {
    string file1 = argv[1];
    string file2 = argv[2];

    compareFiles(file1, file2);

    return 0;
}
