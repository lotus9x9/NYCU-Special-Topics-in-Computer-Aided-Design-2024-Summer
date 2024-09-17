#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <set>

using namespace std;

// 函数：分割字符串
vector<string> split(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter)) {
        token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end());
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

// 函数：提取延迟值和路径节点
bool extractDelayAndPath(const string& line, double& delay, set<string>& path_nodes) {
    size_t delay_pos = line.find("delay = ");
    size_t path_pos = line.find(", the path is: ");

    if (delay_pos == string::npos || path_pos == string::npos) {
        return false;
    }

    string delay_str = line.substr(delay_pos + 8, path_pos - (delay_pos + 8));
    delay = stod(delay_str);

    string path_str = line.substr(path_pos + 15);
    vector<string> nodes = split(path_str, '-');

    for (const auto& node : nodes) {
        path_nodes.insert(node);
    }

    return true;
}

// 函数：比较两个文件
void compareFiles(const string& file1, const string& file2) {
    ifstream inputFile1(file1);
    ifstream inputFile2(file2);

    if (!inputFile1.is_open() || !inputFile2.is_open()) {
        cerr << "Error opening files." << endl;
        return;
    }

    string line1, line2;
    double delay1, delay2;
    set<string> path_nodes1, path_nodes2;
    bool filesAreEquivalent = true;

    // 比较最长延迟
    getline(inputFile1, line1);
    getline(inputFile2, line2);
    if (extractDelayAndPath(line1, delay1, path_nodes1) && extractDelayAndPath(line2, delay2, path_nodes2)) {
        if (fabs(delay1 - delay2) > 1e-6 || path_nodes1 != path_nodes2) {
            cout << "Longest delay differs:\nFile1: " << line1 << "\nFile2: " << line2 << endl;
            filesAreEquivalent = false;
        }
    } else {
        cout << "Error parsing the files for the longest delay." << endl;
        filesAreEquivalent = false;
    }

    // 清除集合以便比较最短延迟
    path_nodes1.clear();
    path_nodes2.clear();

    // 比较最短延迟
    getline(inputFile1, line1);
    getline(inputFile2, line2);
    if (extractDelayAndPath(line1, delay1, path_nodes1) && extractDelayAndPath(line2, delay2, path_nodes2)) {
        if (fabs(delay1 - delay2) > 1e-6 || path_nodes1 != path_nodes2) {
            cout << "Shortest delay differs:\nFile1: " << line1 << "\nFile2: " << line2 << endl;
            filesAreEquivalent = false;
        }
    } else {
        cout << "Error parsing the files for the shortest delay." << endl;
        filesAreEquivalent = false;
    }

    if (filesAreEquivalent) {
        cout << "The files are equivalent." << endl;
    } else {
        cout << "The files are different." << endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <file1> <file2>" << endl;
        return 1;
    }

    compareFiles(argv[1], argv[2]);

    return 0;
}
