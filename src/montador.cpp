#include <bits/stdc++.h>

using namespace std;

// variável global para indicar se o programa tem que ser ligado ou não
bool has_link = false;

// trim from start (in place)
inline void ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(string &s) {
    rtrim(s);
    ltrim(s);
}

void build_instruction_table(map<string, pair<string, int>>& instruction_table) {
    // primeiro elemento do pair: opcode
    // segundo elemento do pair: número de operandos
    instruction_table = {{"ADD", {"1", 2}}, {"SUB", {"2", 2}}, {"MUL", {"3", 2}},
                         {"MULT", {"3", 2}}, {"DIV", {"4", 2}}, {"JMP", {"5", 2}}, 
                         {"JMPN", {"6", 2}}, {"JMPP", {"7", 2}}, {"JMPZ", {"8", 2}}, 
                         {"COPY", {"9", 3}}, {"LOAD", {"10", 2}}, {"STORE", {"11", 2}}, 
                         {"INPUT", {"12", 2}}, {"OUTPUT", {"13", 2}}, {"STOP", {"14", 1}}};
    return;
}

map<int, int*> transform_symbol_table(map<string, pair<pair<int, int*>, bool>>& symbol_table) {
    map<int, int*> m;
    for (auto s : symbol_table) 
        m.insert({s.second.first.first, s.second.first.second});

    return m;
}

void pre_processing(string& file_name, ifstream& file) {
    string line, delimiter = " ";
    bool macro = false, text = false;
    vector<string> pre_processed_code, text_code, data_code, macro_definition_table;
    map<string, int> macro_name_table;

    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        transform(line.begin(), line.end(), line.begin(), ::toupper);

        if (line.find("SECTION TEXT") != line.npos) { 
            text = true;
            continue;
        } 
        
        if (line.find("SECTION DATA") != line.npos) {
            text = false;
            continue;
        }

        line += " ";

        string token;
        vector<string> tokens;
        size_t pos, comma;
        bool is_pointer_arithmetic = false, comment = false;

        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);
            trim(token);

            if (token.size() == 0) {
                line.erase(0, pos+delimiter.length());
                continue;
            }

            // descartando os comentários
            if (token.find(";") != token.npos) {
                token = token.substr(0, token.find(";"));
                comment = true;
            }

            if (is_pointer_arithmetic) {
                tokens[tokens.size()-1] += token;
                is_pointer_arithmetic = false;
            } else if (token.find("+") != token.npos) {
                if (token == "+") {
                    tokens[tokens.size()-1] += token;
                    is_pointer_arithmetic = true;
                } else if (token.substr(0, token.find("+")).size() > 0) {
                    if (token.substr(token.find("+")+1).size() == 0) {
                        is_pointer_arithmetic = true;
                    }
                    tokens.push_back(token);
                } else {
                    tokens[tokens.size()-1] += token;
                }
            } else {
                if (token.find(",") != token.npos) {
                    string t;
                    while ((comma = token.find(",")) != token.npos) {
                        t = token.substr(0, comma);
                        trim(t);
                    
                        if (t.size() == 0) {
                            token.erase(0, comma+1);
                            continue;
                        }

                        tokens.push_back(t);
                        token.erase(0, comma+1);
                    }
                }

                if (token.size()) {
                    tokens.push_back(token);
                }
            }

            if (comment) {
                break;
            }

            line.erase(0, pos+delimiter.length());
        }

        if (macro) {
            string temp = "";

            for (auto t : tokens) {
                temp += t+" ";
            }

            macro_definition_table.push_back(temp);
            
            if (temp.find("ENDMACRO") != temp.npos) {
                macro = false;
            }

            continue;
        }

        if (tokens.size() > 1 && tokens[1].find("MACRO") != tokens[1].npos) {
            string macro_name = tokens[0].substr(0, tokens[0].size()-1);

            if (macro_name_table.count(macro_name) == 1) {
                cerr << "Erro! Macro redefinida!\n";
            } else {
                macro_name_table.insert({macro_name, macro_definition_table.size()});
                macro = true;
            }
           
            continue;
        } 

        if (macro_name_table.count(tokens[0]) == 1) {
            // ainda sem tratar os parametros das macros
            int idx = macro_name_table[tokens[0]];
           
            while (macro_definition_table[idx].find("ENDMACRO") == macro_definition_table[idx].npos) {
                pre_processed_code.push_back(macro_definition_table[idx]);
                idx++;
            }
        } else {
            // código reusado
            string temp = "";
            
            for (auto t : tokens) {
                temp += t+" ";
            }
            
            if (text) {
                text_code.push_back(temp);
            } else {
                data_code.push_back(temp);
            }
        }
    }

    for (string l : text_code) 
        pre_processed_code.push_back(l);
    
    for (string l : data_code) 
        pre_processed_code.push_back(l);

    string pre_processed_file_name = file_name.substr(0, file_name.size()-4) + ".pre";
    ofstream pre_file("./" + pre_processed_file_name);
    ostream_iterator<string> pre_processed_file_iterator(pre_file, "\n");

    if (pre_file.is_open()) {
        copy(begin(pre_processed_code), end(pre_processed_code), pre_processed_file_iterator);
    }

    pre_file.close();

    return;
}

pair<map<string, pair<pair<int, int*>, bool>>, map<string, pair<int, int*>>> first_pass(ifstream& pre_processed_code) {
    int position_counter = 0, line_counter = 1;
    string delimiter = " ", line;
    map<string, pair<pair<int, int*>, bool>> symbol_table;
    map<string, pair<int, int*>> definitions_table;
    map<string, pair<string, int>> instruction_table;
    map<string, int> directives_table = {{"CONST", 1}, {"SPACE", 1}};

    build_instruction_table(instruction_table);

    while (getline(pre_processed_code, line)) {
        vector<string> tokens;
        size_t pos = 0;
        string token, label;

        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);
            tokens.push_back(token);
            line.erase(0, pos+delimiter.length());
        }

        if (tokens[0] == "PUBLIC") {
            definitions_table.insert({tokens[1], {0, (int*) malloc(4)}});
            continue;
        }

        if (tokens[0].find(':') != tokens[0].npos) {
            // caso que existe rótulo na linha
            tokens[0].erase(tokens[0].find(':'));
            label = tokens[0];

            if (isdigit(label[0])) {
                cerr << "Erro na linha " << line_counter << ": erro léxico na criação do rótulo!\n";
                break;
            } else {
                bool error = false;
                for (auto c : label) {
                    if (!(isalnum(c) || c == '_')) {
                        cerr << "Erro na linha " << line_counter << ": erro léxico na criação do rótulo!\n";
                        error = true;
                    }
                }

                if (error) {
                    break;
                } 
            }

            if (symbol_table.count(label) == 1) {
                cerr << "Erro na linha " << line_counter << ": rótulo redefinido!\n";
                break;
            } else {
                symbol_table.insert({label, {{position_counter, (int*) malloc(4)}, 0}});
            }
            
            tokens.erase(tokens.begin());
        }

        if (tokens.size() == 0) {
            continue;
        }

        string operation = tokens[0];

        if (operation.find(":") != operation.npos) {
            cerr << "Erro na linha " << line_counter << ": rótulo dobrado na mesma linha!\n";
            break;
        }

        if (operation == "BEGIN") {
            has_link = true;
            continue;
        }

        if (operation == "END") {
            continue;
        }

        if (operation == "EXTERN") {
            symbol_table[label].second = true;
            continue;
        }   
        
        if (instruction_table.count(operation) == 1) {
            if (tokens.size() != instruction_table[operation].second) { 
                cerr << "Erro na linha " << line_counter << ": número de operandos errados para a instrução!\n";
                break;
            }
            position_counter += instruction_table[operation].second;
        } else {
            if (directives_table.count(operation) == 1) {
                if (tokens.size() > 2) {
                    cerr << "Erro na linha " << line_counter << ": número de operandos errados para a diretiva!\n";
                    break;
                }

                if (operation == "CONST") {
                    if (tokens[1].find("0x") != tokens[1].npos) {
                        int num = 0;
                        for (int i = tokens[1].size()-1; tokens[1][i] != 'x'; i--) {
                            num += (int)pow(16, tokens[1].size()-1-i) * stoi(to_string(tokens[1][i]));
                        }
                        *symbol_table[label].first.second = num;
                    } else{ 
                        *symbol_table[label].first.second = stoi(tokens[1]);                   
                    }
                    position_counter += directives_table[operation];
                } else {
                    if (tokens.size() == 2) {
                        symbol_table[label] = {symbol_table[label].first, (int*) malloc(4*stoi(tokens[1]))};
                        position_counter += stoi(tokens[1]);
                    } else {
                        position_counter += directives_table[operation];
                    }
                }
            } else {
                cerr << "Erro na linha " << line_counter << ": operação não identificada!\n";
                break;
            }
        }

        line_counter++;
    }

    for (auto d : definitions_table) {
        definitions_table[d.first] = symbol_table[d.first].first;
    }

    return {symbol_table, definitions_table};     // retornando a tabela de símbolos para a segunda passagem
}


map<string, vector<int>> second_pass(string& file_name, ifstream& pre_processed_code, map<string, pair<pair<int, int*>, bool>> st, map<int, int*>& symbol_table, map<string, pair<int, int*>>& definitions_table) {
    int position_counter = 0, line_counter = 1;
    string delimiter = " ", machine_code = "", line, bit_map = "";
    map<string, pair<string, int>> instruction_table;
    map<string, vector<int>> usage_table;
    map<string, int> directives_table = {{"CONST", 1}, {"SPACE", 1}};

    build_instruction_table(instruction_table);

    while (getline(pre_processed_code, line)) {
        vector<string> tokens;
        size_t pos = 0;
        string token;

        if (has_link && (line.find("PUBLIC") != line.npos || line.find("EXTERN") != line.npos)) {
            continue;
        }

        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);

            if (token.find(",") != token.npos) {
                token.erase(token.find(","));
            }

            if (token.find(':') == token.npos) {
                tokens.push_back(token);
            } else {
                token.erase(token.size()-1);
            }

            line.erase(0, pos+delimiter.length());
        }

        string operation = tokens[0];

        if (instruction_table.count(operation) == 1) {
            machine_code += instruction_table[tokens[0]].first + " ";
            bit_map += "0 ";

            for (int i = 1; i < tokens.size(); i++) {
                size_t plus = tokens[i].find("+"); 

                if (plus != tokens[i].npos && st.count(tokens[i].substr(0, plus)) == 1) {
                    string label = tokens[i].substr(0, plus);
                    machine_code += to_string(st[label].first.first + stoi(tokens[i].substr(plus+1))) + " ";

                    if (has_link && st[label].second) {
                        bit_map += "0 ";
                        if (usage_table.count(label) == 0) {
                            usage_table.insert({label, {position_counter+i}});
                        } else {
                            usage_table[label].push_back(position_counter+i);
                        }
                    } else {
                        bit_map += "1 ";
                    } 
                } else if (st.count(tokens[i]) == 1) {
                    string label = tokens[i];
                    machine_code += to_string(st[label].first.first) + " ";

                    if (has_link && st[label].second) {
                        bit_map += "0 ";
                        if (usage_table.count(label) == 0) {
                            usage_table.insert({label, {position_counter+i}});
                        } else {
                            usage_table[label].push_back(position_counter+i);
                        }
                    } else {
                        bit_map += "1 ";
                    }
                } else {
                    cerr << "Erro na linha " << line_counter << ": rótulo ausente!\n";
                }
            }
            position_counter += instruction_table[operation].second;
        } else {
            if (directives_table.count(operation) == 1) {
                if (tokens[0] == "SPACE") {
                    if (tokens.size() > 1) {
                        for (int i = 0; i < stoi(tokens[1]); i++) {
                            machine_code += "0 ";
                            bit_map += "0 ";
                        }
                    } else {
                        machine_code += "0 ";
                        bit_map += "0 ";
                    }
                } else {
                    bit_map += "0 ";
                    machine_code += to_string(*symbol_table[position_counter]) + " ";
                }
                position_counter += directives_table[tokens[0]];
            } else {
                if (operation == "BEGIN" || operation == "END") {
                    continue;
                } else {
                    cout << "Erro na linha " << line_counter << "! Operação não indentificada!\n";   
                }
            }
        }

        line_counter++;
    }

    string temp, obj_code_name = file_name.substr(0, file_name.size()-4) + ".obj";
    ofstream obj_code("./" + obj_code_name);

    if (has_link) {
        for (auto d : definitions_table) {
            temp = "D, " + d.first + " " + to_string(d.second.first);
            obj_code << temp + "\n";
        }

        for (auto u : usage_table) {
            for (int i = 0; i < u.second.size(); i++) {
                temp = "U, " + u.first + " " + to_string(u.second[i]);
                obj_code << temp + "\n";    
            }
        }

        obj_code << "R, " + bit_map + "\n";
    }

    if (obj_code.is_open()) {
        obj_code << machine_code;
    }

    obj_code.close();

    return usage_table;
}


int main(int argc, char* argv[]) {
    string file_name = argv[1];
   
    ifstream input_file(file_name);

    if (!input_file.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << file_name << '\n';
        return 1;
    }
    
    if (file_name.substr(file_name.size()-4, 4) == ".asm") {
        pre_processing(file_name, input_file);
    } else if (file_name.substr(file_name.size()-4, 4) == ".pre") {
        map<string, pair<pair<int, int*>, bool>> symbol_table;
        map<string, pair<int, int*>> definitions_table;
        map<string, vector<int>> usage_table;

        tie(symbol_table, definitions_table) = first_pass(input_file);
        map<int, int*> st = transform_symbol_table(symbol_table);
        
        ifstream input_file(file_name);
        usage_table = second_pass(file_name, input_file, symbol_table, st, definitions_table);
        
        input_file.close();
    } else {
        cerr << "Erro: arquivo(s) inválido(s)!\n";
        return 1;
    }

    return 0;
}