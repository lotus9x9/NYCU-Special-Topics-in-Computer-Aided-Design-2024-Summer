#include <bits/stdc++.h>
#include "lib.cpp"
using namespace std;

struct Gate {
    string name;
    string type;
    // port
    string I;
    string A1, A2;
    string ZN;
    // output loading
    double outload = 0;
    // timing 
    bool output = false;
    double total_delay = 0;
    double delay_for_step2 = 0;
    double trans_t_for_step2 = 0;
    //step 4
    double total_delay_step4 = 0;
    double delay_for_step4 = 0;
    double trans_t_for_step4 = 0;
    // Constructor for INV
    Gate(string nm, string ty, string zn, string i) {
        name = nm;
        type = ty;
        ZN = zn;
        I = i;
    }

    // Constructor for XAND XOR
    Gate(string nm, string ty, string zn, string x, string y) {
        name = nm;
        type = ty;
        ZN = zn;
        A1 = x;
        A2 = y;
    }
};

struct Wire {
    int type; 
    vector<Gate*> input_Gates; 
    Gate* output_gate = nullptr; 
    //for step 2
    double trans_t = 0;
    double delay = 0;
    bool visited = false; 
    string previous;
    //for step 4
    bool logic = false;
    double trans_t_for_step_4 = 0;
    double delay_for_step_4 = 0;
    // Constructor
    Wire(int t) : type(t) {
        if (type == 1) {
            visited = true;
            previous = "input";
        }
    }
};

struct Pattern {
    unordered_map<string, bool> inputs; 
};


bool compareGates(const Gate* a, const Gate* b) {
    return stoi(a->name.substr(1)) < stoi(b->name.substr(1));
}


class Circuit {
private:
    unordered_map<string, Wire*> nets;
    vector<Gate*> gates;
    vector<Gate*> sorted_gates; 
    string longest_path;
    double longest_delay;
    string shortest_path;
    double shortest_delay;
    vector<Pattern> pat;
public:
    vector<string> split(const string &str, const char ch) {
        vector<string> result;
        string token;
        for (char c : str) {
            if (isspace(c) || c == ch) {
                if (!token.empty()) {
                    result.push_back(token);
                    token.clear();
                }
            }else token += c;
        }
        if (!token.empty()) result.push_back(token);
        return result;
    }

    string get_gate_name(const string &str) {
        size_t pos = str.find('(');
        string name = str.substr(0, pos);
        name.erase(remove_if(name.begin(), name.end(), ::isspace), name.end());
        return name;
    }

    string get_input_pin(const string &str, const string &from) {
        size_t start = str.find(from + "(");
        if (start == string::npos) return "";
        start += from.length() + 1; 
        size_t end = str.find(')', start);
        return str.substr(start, end - start);
    }
    void processGate(const string& netlist, const string& gateType, const string& gateTypeKeyword, size_t keywordLength) {
        string tmp = netlist.substr(netlist.find(gateTypeKeyword) + keywordLength);
        string s = tmp.substr(0, tmp.find(";"));
        string gate_name = get_gate_name(s);
        string ZN = get_input_pin(s, "ZN");

        if (gateType == "INV") {
            string I = get_input_pin(s, "I");
            Gate* gate = new Gate(gate_name, gateType, ZN, I);
            gates.push_back(gate);
            nets[ZN]->output_gate = gate;
            nets[I]->input_Gates.push_back(gate);
        } else if (gateType == "NAND" || gateType == "NOR") {
            string A1 = get_input_pin(s, "A1");
            string A2 = get_input_pin(s, "A2");
            Gate* gate = new Gate(gate_name, gateType, ZN, A1, A2);
            gates.push_back(gate);
            nets[ZN]->output_gate = gate;
            nets[A1]->input_Gates.push_back(gate);
            nets[A2]->input_Gates.push_back(gate);
        }
    }

    void processNets(const string& netlist, const string& netType, int wireType) {
        string tmp = netlist.substr(netlist.find(netType) + 6 - wireType);
        tmp = tmp.substr(0, tmp.find(";"));
        vector<string> net_names = split(tmp, ',');
        for (const auto & it : net_names) {
            Wire* wire = new Wire(wireType);
            nets[it] = wire;
        }
    }

    void parseVerilogFile(string filename){
        ifstream input(filename);
        string netlist;
        while(getline(input,netlist)){
            size_t startPos;
            while ((startPos = netlist.find("/*")) != std::string::npos) {
                size_t endPos = netlist.find("*/", startPos + 2);
                if (endPos != std::string::npos && endPos > startPos) {
                    netlist = netlist.substr(0, startPos) + netlist.substr(endPos + 2);
                } else {
                    netlist = netlist.substr(0, startPos);
                    std::string nextLine;
                    while (getline(input, nextLine)) {
                        endPos = nextLine.find("*/");
                        if (endPos != std::string::npos) {
                            netlist += nextLine.substr(endPos + 2);
                            break;
                        }
                    }
                }
            }
            size_t commentPos = netlist.find("//");
            if (commentPos != string::npos) {
                netlist = netlist.substr(0, commentPos);
            }
            if (netlist.empty()) continue;
            if (netlist.find("output") != string::npos) processNets(netlist, "output", 0);
            else if (netlist.find("input") != string::npos) processNets(netlist, "input", 1);
            else if (netlist.find("wire") != string::npos) processNets(netlist, "wire", 2);
            if (netlist.find("NOR2X1") != string::npos) processGate(netlist, "NOR", "NOR2X1", 6);
            else if (netlist.find("NANDX1") != string::npos) processGate(netlist, "NAND", "NANDX1", 6);
            else if (netlist.find("INVX1") != string::npos) processGate(netlist, "INV", "INVX1", 5);
        }
    }
    void cal_load(const lib& library) {
        for (Gate* gate : gates) {
            string& name = gate->ZN;
            Wire* outputWire = nets[name];
            if (outputWire->type == 0) {
                gate->outload = 0.030000;
                if(outputWire->input_Gates.size() == 0) continue;
            }
            for (Gate* connectedGate : outputWire->input_Gates) {
                if (connectedGate->I == name) {
                    gate->outload += library.inv_input_cap;
                } else if (connectedGate->A1 == name) {
                    if (connectedGate->type == "NOR") {
                        gate->outload += library.nor2_input_cap[0];
                    } else if (connectedGate->type == "NAND") {
                        gate->outload += library.nand2_input_cap[0];
                    }
                } else if (connectedGate->A2 == name) {
                    if (connectedGate->type == "NOR") {
                        gate->outload += library.nor2_input_cap[1];
                    } else if (connectedGate->type == "NAND") {
                        gate->outload += library.nand2_input_cap[1];
                    }
                }
            }
        }
    }
    void output_step1(const string& filename_step1){
        ofstream output_load(filename_step1);
        for (const auto& gate : gates) {
            output_load << gate->name << " " << fixed << setprecision(6) << gate->outload << "\n";
        }
        output_load.close();
    }

    void topology_sort() {
        queue<Gate*> processingQueue;
        unordered_map<string, int> inDegree;
        for (auto& gate : gates) {
            int count = 0;
            if (gate->type == "INV") {
                if (!nets[gate->I]->visited) {
                    count++;
                }
            } else if (gate->type == "NAND" || gate->type == "NOR") {
                if (!nets[gate->A1]->visited) {
                    count++;
                }
                if (!nets[gate->A2]->visited) {
                    count++;
                }
            }
            inDegree[gate->name] = count;
            if (count == 0) {
                processingQueue.push(gate);
            }
        }
        while (!processingQueue.empty()) {
            Gate* currentGate = processingQueue.front();
            processingQueue.pop();
            sorted_gates.push_back(currentGate);
            nets[currentGate->ZN]->visited = true;
            for (Gate* nextGate : nets[currentGate->ZN]->input_Gates) {
                if (nextGate->I == currentGate->ZN) {
                    inDegree[nextGate->name]--;
                } else if (nextGate->A1 == currentGate->ZN || nextGate->A2 == currentGate->ZN) {
                    inDegree[nextGate->name]--;
                }
                if (inDegree[nextGate->name] == 0) {
                    processingQueue.push(nextGate);
                }
            }
        }
    }

    void interpolation_step2(string type, double cap, int cap_idx, double trans_t, int trans_idx, double& output_trans, double& output_delay, bool &output, lib& lib){ // type 0=nand 1=nor 2=inv
        double result1,result0;
        double x1 = lib.output_cap[cap_idx];
        double x2 = lib.output_cap[cap_idx+1]; 
        double y2 = lib.input_trans_time[trans_idx]; 
        double y1 = lib.input_trans_time[trans_idx+1];
        double pol1, pol2;
        double time0, time1, time2, time3;
        if(type=="NOR"){ 
            time0 = lib.nor2_rise_time[trans_idx][cap_idx];   
            time1 = lib.nor2_rise_time[trans_idx][cap_idx+1];  
            time2 = lib.nor2_rise_time[trans_idx+1][cap_idx];  
            time3 = lib.nor2_rise_time[trans_idx+1][cap_idx+1]; 
            pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
            pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
            result1 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            time0 = lib.nor2_fall_time[trans_idx][cap_idx];
            time1 = lib.nor2_fall_time[trans_idx][cap_idx+1];
            time2 = lib.nor2_fall_time[trans_idx+1][cap_idx];
            time3 = lib.nor2_fall_time[trans_idx+1][cap_idx+1];
            pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
            pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
            result0 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            output_delay = (result1 >= result0) ? result1 : result0;
            output = (result1 >= result0);
            if(output){
                time0 = lib.nor2_rise_trans_time[trans_idx][cap_idx];   
                time1 = lib.nor2_rise_trans_time[trans_idx][cap_idx+1];  
                time2 = lib.nor2_rise_trans_time[trans_idx+1][cap_idx];  
                time3 = lib.nor2_rise_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                result1 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                output_trans = result1;
            }
            else{
                time0 = lib.nor2_fall_trans_time[trans_idx][cap_idx];
                time1 = lib.nor2_fall_trans_time[trans_idx][cap_idx+1];
                time2 = lib.nor2_fall_trans_time[trans_idx+1][cap_idx];
                time3 = lib.nor2_fall_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                result0 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                output_trans = result0;
            }
        }
        else if(type=="NAND"){ 
            time0 = lib.nand2_rise_time[trans_idx][cap_idx];   
            time1 = lib.nand2_rise_time[trans_idx][cap_idx+1];  
            time2 = lib.nand2_rise_time[trans_idx+1][cap_idx];  
            time3 = lib.nand2_rise_time[trans_idx+1][cap_idx+1]; 
            pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
            pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
            result1 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            time0 = lib.nand2_fall_time[trans_idx][cap_idx];
            time1 = lib.nand2_fall_time[trans_idx][cap_idx+1];
            time2 = lib.nand2_fall_time[trans_idx+1][cap_idx];
            time3 = lib.nand2_fall_time[trans_idx+1][cap_idx+1];
            pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
            pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
            result0 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            output_delay = (result1 >= result0) ? result1 : result0;
            output = (result1 >= result0);
            if(output){
                time0 = lib.nand2_rise_trans_time[trans_idx][cap_idx];  
                time1 = lib.nand2_rise_trans_time[trans_idx][cap_idx+1];  
                time2 = lib.nand2_rise_trans_time[trans_idx+1][cap_idx];  
                time3 = lib.nand2_rise_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                result1 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                output_trans = result1;
            }
            else{
                time0 = lib.nand2_fall_trans_time[trans_idx][cap_idx];
                time1 = lib.nand2_fall_trans_time[trans_idx][cap_idx+1];
                time2 = lib.nand2_fall_trans_time[trans_idx+1][cap_idx];
                time3 = lib.nand2_fall_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                result0 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                output_trans = result0;
            }
        }
        else if(type=="INV"){ 
            time0 = lib.inv_rise_time[trans_idx][cap_idx];   
            time1 = lib.inv_rise_time[trans_idx][cap_idx+1];  
            time2 = lib.inv_rise_time[trans_idx+1][cap_idx];  
            time3 = lib.inv_rise_time[trans_idx+1][cap_idx+1]; 
            pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
            pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
            result1 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            time0 = lib.inv_fall_time[trans_idx][cap_idx];
            time1 = lib.inv_fall_time[trans_idx][cap_idx+1];
            time2 = lib.inv_fall_time[trans_idx+1][cap_idx];
            time3 = lib.inv_fall_time[trans_idx+1][cap_idx+1]; 
            pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
            pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
            result0 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            output_delay = (result1 >= result0) ? result1 : result0;
            output = (result1 >= result0);
            if(output){
                time0 = lib.inv_rise_trans_time[trans_idx][cap_idx]; 
                time1 = lib.inv_rise_trans_time[trans_idx][cap_idx+1];  
                time2 = lib.inv_rise_trans_time[trans_idx+1][cap_idx]; 
                time3 = lib.inv_rise_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                result1 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                output_trans = result1;
            }
            else{
                time0 = lib.inv_fall_trans_time[trans_idx][cap_idx];
                time1 = lib.inv_fall_trans_time[trans_idx][cap_idx+1];
                time2 = lib.inv_fall_trans_time[trans_idx+1][cap_idx];
                time3 = lib.inv_fall_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                result0 = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                output_trans = result0;
            }
        }
    }
    void calculate_trans_delay(lib& lib) {
        for (Gate* gate : sorted_gates) {
            string ZN = gate->ZN;
            double tmp_trans, tmp_delay;
            bool output;
            if (gate->type == "NAND" || gate->type == "NOR") {
                handle_nand_nor(gate, lib, tmp_trans, tmp_delay, output);
            } else if (gate->type == "INV") {
                handle_inv(gate, lib, tmp_trans, tmp_delay, output);
            }
            gate->delay_for_step2 = tmp_delay;
            gate->trans_t_for_step2 = tmp_trans;
            gate->output = output;
            nets[ZN]->delay = gate->total_delay;
            nets[ZN]->trans_t = tmp_trans;
            nets[ZN]->previous = (gate->type == "INV") ? gate->I : (nets[gate->A1]->delay > nets[gate->A2]->delay) ? gate->A1 : gate->A2;
        }
    }

    void handle_nand_nor(Gate* gate, lib& lib, double& tmp_trans, double& tmp_delay, bool& output) {
        string A1 = gate->A1;
        string A2 = gate->A2;
        double trans = (nets[A1]->delay > nets[A2]->delay) ? nets[A1]->trans_t : nets[A2]->trans_t;
        double delay = (nets[A1]->delay > nets[A2]->delay) ? nets[A1]->delay : nets[A2]->delay;
        int col = find_index(lib.output_cap, gate->outload);
        int row = find_index(lib.input_trans_time, trans);
        interpolation_step2(gate->type, gate->outload, col, trans, row, tmp_trans, tmp_delay, output, lib);
        gate->total_delay = delay + tmp_delay + 0.005;
    }

    void handle_inv(Gate* gate, lib& lib, double& tmp_trans, double& tmp_delay, bool& output) {
        string I = gate->I;
        int col = find_index(lib.output_cap, gate->outload);
        int row = find_index(lib.input_trans_time, nets[I]->trans_t);
        interpolation_step2("INV", gate->outload, col, nets[I]->trans_t, row, tmp_trans, tmp_delay, output, lib);
        gate->total_delay = nets[I]->delay + tmp_delay + 0.005;
    }

    int find_index(const double* array, double value) {
        auto it = lower_bound(array, array + 7, value);
        int index = distance(array, it) - 1;
        if (index < 0) index = 0;
        else if (index >= 6) index = 5;
        return index;
    }

    void output_step2(const string& output_file) {
        ofstream output_delay(output_file);
        for (const auto& gate : sorted_gates) {
            string gate_name = gate->name;
            int worst_output = gate->output ? 1 : 0;
            double propagation_delay = gate->delay_for_step2;
            double transition_time = gate->trans_t_for_step2;
            output_delay << gate_name << " " << worst_output << " "<< fixed << setprecision(6) << propagation_delay << " "<< fixed << setprecision(6) << transition_time << endl;
        }
        output_delay.close();
    }
    void find_longest_delay_path() {
        longest_delay = -1.0;
        string end_net = "";
        for (const auto & it : nets) {
            Wire* wire = it.second;
            if (wire->type == 0) {  
                if (wire->delay > longest_delay) {
                    longest_delay = wire->delay;
                    end_net = it.first;
                }
            }
        }
        longest_path = end_net;
        while (nets[end_net]->previous != "input") {
            end_net = nets[end_net]->previous;
            longest_path = end_net + " -> " + longest_path;
        }
        longest_delay=longest_delay - 0.005;
    }
    void find_shortest_delay_path() {
        shortest_delay = DBL_MAX;
        string end_net = "";
        for (const auto & it : nets) {
            Wire* wire = it.second;
            if (wire->type == 0) {  
                if (wire->delay < shortest_delay) {
                    shortest_delay = wire->delay;
                    end_net = it.first;
                }
            }
        }
        shortest_path = end_net;
        while (nets[end_net]->previous != "input") {
            end_net = nets[end_net]->previous;
            shortest_path = end_net + " -> " + shortest_path;
        }
        shortest_delay=shortest_delay - 0.005;
    }
    void output_step3(const string& filename_path) {
        ofstream output_file(filename_path);
        output_file << "Longest delay = " << fixed << setprecision(6) << longest_delay << ", the path is: " << longest_path << endl;
        output_file << "Shortest delay = " << fixed << setprecision(6) << shortest_delay << ", the path is: " << shortest_path << endl;
        output_file.close();
    }
    void parse_pat_file(string filename) {
        ifstream input(filename);
        string line;
        vector<string> input_names;
        while (getline(input, line)) {
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            if (line.empty() || line.find("input") != string::npos) {
                istringstream iss(line.substr(5)); 
                string name;
                while (iss >> name) {
                    name.erase(remove_if(name.begin(), name.end(), [](char c) { return isspace(c) || c == ','; }), name.end());
                    if (!name.empty()) {
                        input_names.push_back(name);
                    }
                }
                break;
            }
        }
        while (getline(input, line)) {
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            if (line == ".end") break;
            istringstream iss(line);
            Pattern pattern;
            for (const string &input_name : input_names) {
                string value_str;
                if (iss >> value_str) {
                    value_str.erase(remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
                    if (!value_str.empty() && (value_str == "0" || value_str == "1")) {
                        pattern.inputs[input_name] = (value_str == "1");
                    }
                }
            }
            pat.push_back(pattern);
        }
    }
    void initialize_input_nets(const Pattern& pattern) {
        for (unordered_map<string, bool>::const_iterator it = pattern.inputs.begin(); it != pattern.inputs.end(); it++) {
            const string& input_name = it->first;
            bool input_value = it->second;
            if (nets.find(input_name) != nets.end()) {
                nets[input_name]->logic = input_value;
            }
        }
    }

    void calculate_gate_output(Gate* gate) {
        if (gate->type == "INV") {
            nets[gate->ZN]-> logic = !nets[gate->I]->logic;
        } else if (gate->type == "NAND") {
            nets[gate->ZN]-> logic = !(nets[gate->A1]->logic && nets[gate->A2]->logic);
        } else if (gate->type == "NOR") {
            nets[gate->ZN]-> logic = !(nets[gate->A1]->logic || nets[gate->A2]->logic);
        }
    }

    void interpolation_step4(string type,bool logic, double cap, int cap_idx, double trans_t, int trans_idx, double& output_trans, double& output_delay, lib& lib){ // type 0=nand 1=nor 2=inv
        double x1 = lib.output_cap[cap_idx];
        double x2 = lib.output_cap[cap_idx+1]; 
        double y2 = lib.input_trans_time[trans_idx]; 
        double y1 = lib.input_trans_time[trans_idx+1];
        double pol1, pol2;
        double time0, time1, time2, time3;
        if(type=="NOR"){ 
            if(logic){
                time0 = lib.nor2_rise_time[trans_idx][cap_idx];   
                time1 = lib.nor2_rise_time[trans_idx][cap_idx+1];  
                time2 = lib.nor2_rise_time[trans_idx+1][cap_idx];  
                time3 = lib.nor2_rise_time[trans_idx+1][cap_idx+1]; 
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_delay = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                time0 = lib.nor2_rise_trans_time[trans_idx][cap_idx];   
                time1 = lib.nor2_rise_trans_time[trans_idx][cap_idx+1];  
                time2 = lib.nor2_rise_trans_time[trans_idx+1][cap_idx];  
                time3 = lib.nor2_rise_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_trans = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            }
            else{
                time0 = lib.nor2_fall_time[trans_idx][cap_idx];
                time1 = lib.nor2_fall_time[trans_idx][cap_idx+1];
                time2 = lib.nor2_fall_time[trans_idx+1][cap_idx];
                time3 = lib.nor2_fall_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_delay = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                time0 = lib.nor2_fall_trans_time[trans_idx][cap_idx];
                time1 = lib.nor2_fall_trans_time[trans_idx][cap_idx+1];
                time2 = lib.nor2_fall_trans_time[trans_idx+1][cap_idx];
                time3 = lib.nor2_fall_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_trans = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            }
        }
        else if(type=="NAND"){ 
            if(logic){
                time0 = lib.nand2_rise_time[trans_idx][cap_idx];   
                time1 = lib.nand2_rise_time[trans_idx][cap_idx+1];  
                time2 = lib.nand2_rise_time[trans_idx+1][cap_idx];  
                time3 = lib.nand2_rise_time[trans_idx+1][cap_idx+1]; 
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_delay = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                time0 = lib.nand2_rise_trans_time[trans_idx][cap_idx];   
                time1 = lib.nand2_rise_trans_time[trans_idx][cap_idx+1];  
                time2 = lib.nand2_rise_trans_time[trans_idx+1][cap_idx];  
                time3 = lib.nand2_rise_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_trans = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            }
            else{
                time0 = lib.nand2_fall_time[trans_idx][cap_idx];
                time1 = lib.nand2_fall_time[trans_idx][cap_idx+1];
                time2 = lib.nand2_fall_time[trans_idx+1][cap_idx];
                time3 = lib.nand2_fall_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_delay = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                time0 = lib.nand2_fall_trans_time[trans_idx][cap_idx];
                time1 = lib.nand2_fall_trans_time[trans_idx][cap_idx+1];
                time2 = lib.nand2_fall_trans_time[trans_idx+1][cap_idx];
                time3 = lib.nand2_fall_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_trans = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            }
        }
        else if(type=="INV"){ 
            if(logic){
                time0 = lib.inv_rise_time[trans_idx][cap_idx];   
                time1 = lib.inv_rise_time[trans_idx][cap_idx+1];  
                time2 = lib.inv_rise_time[trans_idx+1][cap_idx];  
                time3 = lib.inv_rise_time[trans_idx+1][cap_idx+1]; 
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_delay = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                time0 = lib.inv_rise_trans_time[trans_idx][cap_idx];   
                time1 = lib.inv_rise_trans_time[trans_idx][cap_idx+1];  
                time2 = lib.inv_rise_trans_time[trans_idx+1][cap_idx];  
                time3 = lib.inv_rise_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_trans = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            }
            else{
                time0 = lib.inv_fall_time[trans_idx][cap_idx];
                time1 = lib.inv_fall_time[trans_idx][cap_idx+1];
                time2 = lib.inv_fall_time[trans_idx+1][cap_idx];
                time3 = lib.inv_fall_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_delay = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
                time0 = lib.inv_fall_trans_time[trans_idx][cap_idx];
                time1 = lib.inv_fall_trans_time[trans_idx][cap_idx+1];
                time2 = lib.inv_fall_trans_time[trans_idx+1][cap_idx];
                time3 = lib.inv_fall_trans_time[trans_idx+1][cap_idx+1];
                pol1 = (time0-time2)/(y2-y1)*(trans_t-y1)+time2;
                pol2 = (time1-time3)/(y2-y1)*(trans_t-y1)+time3;
                output_trans = (pol2-pol1)/(x2-x1)*(cap-x1)+ pol1;
            }
        }
    }

    void cal_for_step4(const Pattern& pattern,lib& library){
        for (Gate* gate : sorted_gates) {
            calculate_gate_output(gate);
            double input_transition_time = 0.0;
            double delay = 0;
            if (gate->type == "INV") {
                input_transition_time = nets[gate->I]->trans_t_for_step_4;
                delay = nets[gate->I]->delay_for_step_4;
            } else calculate_input_transition_time(gate,input_transition_time,delay);
            int cap_idx = find_index(library.output_cap, gate->outload);
            int trans_idx = find_index(library.input_trans_time, input_transition_time);
            double tmp_trans;
            double tmp_delay;
            interpolation_step4(gate->type,nets[gate->ZN]->logic, gate->outload,cap_idx,input_transition_time,trans_idx, tmp_trans, tmp_delay,library);
            gate->total_delay_step4 = delay + tmp_delay + 0.005;
            gate->delay_for_step4 = tmp_delay;
            gate->trans_t_for_step4 = tmp_trans;
            nets[gate->ZN]->delay_for_step_4 = gate->total_delay_step4;
            nets[gate->ZN]->trans_t_for_step_4 = tmp_trans;
        }
    }

    void calculate_input_transition_time(Gate* gate, double& input_transition_time, double& delay) {
        double delay_controlling = DBL_MAX;
        double delay_non_controlling = 0;
        if (gate->type == "NAND") {
            if (nets[gate->A1]->logic == 0) delay_controlling = min(delay_controlling, nets[gate->A1]->delay_for_step_4);
            if (nets[gate->A2]->logic == 0) delay_controlling = min(delay_controlling, nets[gate->A2]->delay_for_step_4);
            if (nets[gate->A1]->logic == 1) delay_non_controlling = max(delay_non_controlling, nets[gate->A1]->delay_for_step_4);
            if (nets[gate->A2]->logic == 1) delay_non_controlling = max(delay_non_controlling, nets[gate->A2]->delay_for_step_4);
        } else if (gate->type == "NOR") {
            if (nets[gate->A1]->logic == 1) delay_controlling = min(delay_controlling, nets[gate->A1]->delay_for_step_4);
            if (nets[gate->A2]->logic == 1) delay_controlling = min(delay_controlling, nets[gate->A2]->delay_for_step_4);
            if (nets[gate->A1]->logic == 0) delay_non_controlling = max(delay_non_controlling, nets[gate->A1]->delay_for_step_4);
            if (nets[gate->A2]->logic == 0) delay_non_controlling = max(delay_non_controlling, nets[gate->A2]->delay_for_step_4);
        }
        if (delay_controlling < DBL_MAX) {
            input_transition_time = (nets[gate->A1]->delay_for_step_4 == delay_controlling) ? nets[gate->A1]->trans_t_for_step_4 : nets[gate->A2]->trans_t_for_step_4;
            delay = delay_controlling;
        } else {
            input_transition_time = (nets[gate->A1]->delay_for_step_4 == delay_non_controlling) ? nets[gate->A1]->trans_t_for_step_4 : nets[gate->A2]->trans_t_for_step_4;
            delay = delay_non_controlling;
        }
    }

    void output_gate_info_for_pattern(lib& library,const string& output_filename) {
        ofstream output_file(output_filename);
        sort(gates.begin() , gates.end() , compareGates);
        for (const auto & it : pat) {
            initialize_input_nets(it);
            cal_for_step4(it, library);
            for (const auto& gate : gates) {
                output_file << gate->name << " " << nets[gate->ZN]->logic << " " << fixed << setprecision(6) << gate->delay_for_step4 << " " << gate->trans_t_for_step4 << "\n";
            }
            output_file << "\n";
        }
        output_file.close();
    }

};
