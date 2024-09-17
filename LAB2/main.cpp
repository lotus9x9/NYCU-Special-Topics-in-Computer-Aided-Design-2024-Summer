#include <bits/stdc++.h>
#include "circuit.cpp"
using namespace std;


int main(int argc , char** argv){
    clock_t start = clock();
    lib lib;
    Circuit Q;
    string case_name = argv[1];
    size_t last_slash = case_name.find_last_of("/\\");
    size_t last_dot = case_name.find_last_of(".");
    if (last_slash == std::string::npos) {
        last_slash = -1;
    }
    case_name = case_name.substr(last_slash + 1, last_dot - last_slash - 1);
    Q.parseVerilogFile(argv[1]);
    ifstream input_lib(argv[3]);
    Q.cal_load(lib);
    Q.output_step1("312510239_"+case_name+"_load.txt");
    // Q.output_step1("out_load.txt");
    Q.topology_sort();
    Q.calculate_trans_delay(lib);
    Q.output_step2("312510239_"+case_name+"_delay.txt");
    // Q.output_step2("out_delay.txt");
    Q.find_longest_delay_path();
    Q.find_shortest_delay_path();
    Q.output_step3("312510239_"+case_name+"_path.txt");
    // Q.output_step3("out_path.txt");
    Q.parse_pat_file(argv[5]);
    // Q.output_gate_info_for_pattern(lib,"out_gate_info.txt");
    Q.output_gate_info_for_pattern(lib,"312510239_"+case_name+"_gate_info.txt");
    cout << (double)(clock()-start) / CLOCKS_PER_SEC * 1000 << "ms" << '\n';
    return 0;
}