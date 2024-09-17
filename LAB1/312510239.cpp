#include<bits/stdc++.h>

using namespace std;
string binary_to_literal(const string &binary) {
    string literal;
    for (int i = 0; i < binary.size(); i++) {
        if (binary[i] == '1') {
            literal += char('A' + i); // A, B, C, ...
        } else if (binary[i] == '0') {
            literal += char('A' + i);
            literal += "'";
        }
    }
    return literal;
}
// Function to count literals in a binary implicant
int literalcount(const string &implicant) {
    return count_if(implicant.begin(), implicant.end(), [](char c) { return c == '1' || c == '0'; });
}

// Function to compare implicants for sorting
bool literal_compare(const string &a, const string &b) {
    // Compare by literal count first (more literals first)
    int a_literals = literalcount(a);
    int b_literals = literalcount(b);

    if (a_literals != b_literals) {
        return a_literals > b_literals; // More literals should come first
    }

    // Then compare by alphabetical order
    return a > b;
}

// Function to compare implicants for sorting
bool implicant_compare(const string &a, const string &b) {
    string a_lit = binary_to_literal(a);
    string b_lit = binary_to_literal(b);
    return literal_compare(a_lit, b_lit);
}
class Quine_McCluskey
{
private:
    int var_num;
    vector<string>on_set;
    unordered_set<string>dont_care;
    set<string>all_set;
    vector<string> prime_implicants;
    int min_clauses_count;
    vector<vector<string>> notessPrime_imp;
    vector<set<string>> patrick_imp;
    vector<string> finalans;
public:
    //Input File
    void inputfile(string filename){
        ifstream input(filename);
        string tmp;
        int decimal;
        string binary;
        input>>tmp>>var_num;
        input>>tmp;
        while (!input.eof())
        { 
            input>>tmp;
            if (tmp==".d") break;
            decimal=stoi(tmp);
            binary=Decimal_To_Binary(decimal);
            on_set.push_back(binary);
            all_set.insert(binary);
        }
        while (!input.eof())
        {
            input>>decimal;
            binary=Decimal_To_Binary(decimal);
            dont_care.insert(binary);
            all_set.insert(binary);
        }
        input.close();
    }
    //Change decimal to binary
    string Decimal_To_Binary(int decimal) {
        if (decimal == 0) return string(var_num, '0'); 
        string binary = "";
        while (decimal > 0) {
            binary = (decimal % 2 == 0 ? "0" : "1") + binary;
            decimal /= 2;
        }
        while (binary.size() < var_num) {
            binary = "0" + binary;
        }
        return binary;
    }
    //Find prime implicants
    void find_prime_implicants() {
        while (!all_set.empty()) {
            vector<bool> check_list(all_set.size(), false);
            set<string> not_prime_imp;
            string tmp;
            int counta = 0;

            for (auto it1 = all_set.begin(); it1 != all_set.end(); it1++, counta++) {
                int countb = counta + 1;
                for (auto it2 = next(it1); it2 != all_set.end(); it2++, countb++) {
                    int diff=0;
                    tmp=*it1;
                    for( int i=0; i<(*it1).length(); i++ ) {
                        if( (*it1)[i]!=(*it2)[i]){
                            diff++;
                            tmp[i]='-';
                        }
                    }
                    if (diff==1) {
                        check_list[counta] = true;
                        check_list[countb] = true;
                        not_prime_imp.insert(tmp);
                    }
                }
            }

            // Add prime implicants to the list
            for (int i = 0; i < check_list.size(); i++) {
                if (!check_list[i]) {
                    prime_implicants.push_back(*next(all_set.begin(), i));
                }
            }

            // Prepare for the next round
            all_set.clear();
            all_set.insert(not_prime_imp.begin(), not_prime_imp.end());
        }
        return;
    }   
    //檢查兩組string是否能覆蓋
    bool checkPrime(const string& a,const string& b){
        for (int i = 0; i < var_num; i++) {
            if (a[i] != '-' && a[i] != b[i]) {
                return false;
            }
        }
        return true;
    }
    //使用patrick算法來找到minimum cover
    void patrick(set<string>& p_set, int idx, int max, int iter) {
        if (idx == max) {
            if (iter <= min_clauses_count) {
                min_clauses_count = iter;
                patrick_imp.clear(); 
                patrick_imp.push_back(p_set); 
            } else if (iter == min_clauses_count) {
                patrick_imp.push_back(p_set); 
            }
            return;
        }
        if (iter > min_clauses_count) return;
        const auto& implicants = notessPrime_imp[idx];
        for (int i = 0; i < implicants.size(); i++) {
            const string& implicant = implicants[i];
            if (p_set.find(implicant) == p_set.end()) {
                p_set.insert(implicant);
                patrick(p_set, idx + 1, max, iter + 1);
                p_set.erase(implicant);
            }
            else patrick(p_set, idx + 1, max, iter);
        }
        return;
    }

    void minCover() {
        int onset_count = on_set.size();
        int primes_count = prime_implicants.size();

        // 使用 vector 替代动态二维数组
        vector<vector<int>> prime_Imp_chart(primes_count + 1, vector<int>(onset_count + 1, 0));

        // 将 unordered_set 转换为 vector 以便索引访问
        vector<string> prime_imp_vec(prime_implicants.begin(), prime_implicants.end());

        // 计算 essential prime_imp
        for (int i = 0; i < primes_count; i++) {
            for (int j = 0; j < onset_count; j++) {
                if (checkPrime(prime_imp_vec[i], on_set[j])) {    
                    prime_Imp_chart[i][j] = 1;
                    prime_Imp_chart[primes_count][j]++;
                    prime_Imp_chart[i][onset_count]++;
                }
            }
        }

        // 提取 essential prime_imp
        for (int i = 0; i < onset_count; i++) {
            if (prime_Imp_chart[primes_count][i] == 1) {   
                prime_Imp_chart[primes_count][i] = 0;
                for (int j = 0; j < primes_count; j++) {
                    if (prime_Imp_chart[j][i] == 1) {
                        finalans.push_back(prime_imp_vec[j]);
                        prime_Imp_chart[j][onset_count] = 0;
                        for (int k = 0; k < onset_count; k++) {
                            if (prime_Imp_chart[j][k] == 1)
                                prime_Imp_chart[primes_count][k] = 0;
                        }
                        break;
                    }
                }
            }
        }

        // 提取 non-essential prime_imp
        for (int i = 0; i < onset_count; i++) {
            if (prime_Imp_chart[primes_count][i] != 0) {
                vector<string> primes;
                for (int j = 0; j < primes_count; j++) {
                    if (prime_Imp_chart[j][i] != 0) {
                        primes.push_back(prime_imp_vec[j]);
                    }
                }
                notessPrime_imp.push_back(primes);
            }
        }

        // patrick 递归找最小覆盖
        set<string> min_cov;
        min_clauses_count = INT_MAX;
        patrick(min_cov, 0, notessPrime_imp.size(), 0);

        // 查找最小的 patrick_set 和计数
        vector<set<string>> best_solutions;
        int min_patrick_count = INT_MAX;

        for (const auto& solution : patrick_imp) {
            if (solution.size() == min_clauses_count) {
                int count = 0;
                for (const auto& implicant : solution) {
                    count += literalcount(implicant);
                }
                if (count < min_patrick_count) {
                    min_patrick_count = count;
                    best_solutions.clear();
                    best_solutions.push_back(solution);
                } else if (count == min_patrick_count) {
                    best_solutions.push_back(solution);
                }
            }
        }

        // 将所有最优解添加到 finalans
        for (const auto& best_solution : best_solutions) {
            finalans.insert(finalans.end(), best_solution.begin(), best_solution.end());
        }
        return;
    }
    void writeOutput(string filename) {
        ofstream output(filename);
        stringstream streambuf;
        string s;

        // Output prime implicants
        streambuf << prime_implicants.size();
        output << ".p " << streambuf.str() << endl;

        // Sort prime implicants by literal count and then alphabetically
        sort(prime_implicants.begin(), prime_implicants.end(), [](const string &a, const string &b) {
            return literal_compare(a, b);
        });

        int size = prime_implicants.size();
        for (int i = 0; i < min(size, 15); i++) {
            s = binary_to_literal(prime_implicants[i]);
            output << s << endl;
        }
        output << endl;

        streambuf.str("");
        streambuf.clear();
        streambuf << finalans.size();
        output << ".mc " << streambuf.str() << endl;

        // Sort minimum covering implicants by literal count and then alphabetically
        sort(finalans.begin(), finalans.end(), [](const string &a, const string &b) {
            return literal_compare(a, b);
        });

        size = finalans.size();
        int literals = 0;
        for (int i = 0; i < size; i++) {
            s = binary_to_literal(finalans[i]);
            literals += literalcount(finalans[i]);
            output << s << endl;
        }
        output << "literal=" << literals << endl;
    }
};

int main(int argc,char**argv){
    Quine_McCluskey Q;
    Q.inputfile(argv[1]);
    Q.find_prime_implicants();
    Q.minCover();
    Q.writeOutput(argv[2]);
    return 0;
}