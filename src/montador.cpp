#include <bits/stdc++.h>

using namespace std;

// variável global para indicar se o programa tem que ser ligado ou não
bool linker = false;

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
    instruction_table = {{"ADD", {"01", 2}}, {"SUB", {"02", 2}}, {"MUL", {"03", 2}},
                         {"MULT", {"03", 2}}, {"DIV", {"04", 2}}, {"JMP", {"05", 2}}, 
                         {"JMPN", {"06", 2}}, {"JMPP", {"07", 2}}, {"JMPZ", {"08", 2}}, 
                         {"COPY", {"09", 3}}, {"LOAD", {"10", 2}}, {"STORE", {"11", 2}}, 
                         {"INPUT", {"12", 2}}, {"OUTPUT", {"13", 2}}, {"STOP", {"14", 1}}};
    return;
}

map<int, int*> transform_symbol_table(map<string, pair<int, int*>>& symbol_table) {
    map<int, int*> m;
    for (auto s : symbol_table) 
        m.insert({s.second.first, s.second.second});

    return m;
}

ofstream pre_processing(ifstream& file) {
    string line, delimiter = " ";
    bool macro = false, text = false;
    vector<string> pre_processed_code, text_code, data_code, macro_definition_table;
    map<string, int> macro_name_table;

    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        transform(line.begin(), line.end(), line.begin(), ::toupper);

        if (line == "SECTION TEXT") { 
            text = true;
            continue;
        } 
        
        if (line == "SECTION DATA") {
            text = false;
            continue;
        }

        string token;
        vector<string> tokens;
        size_t pos, comma;

        if (line.find(delimiter) == line.npos) {
            tokens.push_back(line);
        } else {
            while ((pos = line.find(delimiter)) != line.npos) {
                token = line.substr(0, pos);
                trim(token);

                // descartando os comentários
                if (token.find(";") != token.npos) {
                    break;
                }

                while ((comma = token.find(",")) != line.npos) {
                    tokens.push_back(token.substr(0, comma));
                    token.erase(0, comma+1);
                }

                if (token.size()) {
                    tokens.push_back(token);
                }

                line.erase(0, pos+delimiter.length());
            }

            if (line.find(";") == line.npos) {
                tokens.push_back(line);
            }
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
                cout << "Erro! Macro redefinida!\n";
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

    ofstream file("./exemplo.pre");
    ostream_iterator<string> file_iterator(file, "\n");

    copy(begin(pre_processed_code), end(pre_processed_code), file_iterator);

    file.close();
    return file;
}

map<string, pair<int, int*>> first_pass(vector<string> pre_processed_code) {
    int position_counter = 0, line_counter = 1;
    string delimiter = " ";
    map<string, pair<int, int*>> symbol_table;
    map<string, pair<string, int>> instruction_table;
    map<string, int> directives_table = {{"CONST", 2}, {"SPACE", 1}};

    build_instruction_table(instruction_table);

    for (string line : pre_processed_code) {
        vector<string> tokens;
        size_t pos = 0;
        string token, label;

        if (line.find("BEGIN") != line.npos || line.find("END") != line.npos) {
            linker = true;
            continue;
        }

        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);
            if (token.find(",") != token.npos) {
                token.erase(token.find(","));
            }
            tokens.push_back(token);
            line.erase(0, pos+delimiter.length());
        }

        if (tokens[0].find(':') != tokens[0].npos) {
            // caso que existe rótulo na linha
            tokens[0].erase(tokens[0].find(':'));
            label = tokens[0];

            if (isdigit(label[0])) {
                cerr << "Erro na linha " << line_counter << ": erro léxico na criação do rótulo!\n";
                continue;
            } else {
                for (auto c : label) {
                    if (!(isalnum(c) || c == '_')) {
                        cerr << "Erro na linha " << line_counter << ": erro léxico na criação do rótulo!\n";
                        continue;
                    }
                }
            }

            if (symbol_table.count(label) == 1) {
                cerr << "Erro na linha " << line_counter << ": rótulo redefinido!\n";
            } else {
                symbol_table.insert({label, {position_counter, (int*) malloc(4)}});
            }
            tokens.erase(tokens.begin());
        }

        if (tokens.size() == 0) {
            continue;
        }

        string operation = tokens[0];
        
        if (instruction_table.count(operation) == 1) {
            // verificando apenas se a quantidade de operandos está correta, 
            // falta verificar a corretude (2a passagem)
            if (tokens.size() != instruction_table[operation].second) { 
                cerr << "Erro na linha " << line_counter << ": número de operandos errados para a instrução!\n";
            }
            position_counter += instruction_table[operation].second;
        } else {
            if (directives_table.count(operation) == 1) {
                // chama subrotina que executa a tarefa TODO
                if (operation == "CONST") {
                    if (tokens[1].find("0x") != tokens[1].npos) {
                        int num = 0;
                        for (int i = tokens[1].size()-1; tokens[1][i] != 'x'; i--) {
                            num += (int)pow(16, tokens[1].size()-1-i) + stoi(to_string(tokens[1][i]));
                        }
                        *symbol_table[label].second = num;
                    } else{ 
                        *symbol_table[label].second = stoi(tokens[1]);                   
                    }
                }
                position_counter += directives_table[operation];
            } else {
                cerr << "Erro na linha " << line_counter << ": operação não identificada!\n";
            }
        }

        line_counter++;
    }

    return symbol_table;     // retornando a tabela de símbolos para a segunda passagem
}


vector<string> second_pass(vector<string> pre_processed_code, map<string, pair<int, int*>> st, map<int, int*>& symbol_table) {
    int position_counter = 0, line_counter = 1;
    string delimiter = " ";
    map<string, pair<string, int>> instruction_table;
    map<string, int> directives_table = {{"CONST", 2}, {"SPACE", 1}};
    vector<string> machine_code;

    build_instruction_table(instruction_table);

    for (string line : pre_processed_code) {
        vector<string> tokens;
        size_t pos = 0;
        string token, line_machine_code = "";

        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);

            if (token.find(",") != token.npos) {
                token.erase(token.find(","));
            }

            if (token.find(':') == token.npos) {
                tokens.push_back(token);
            }

            line.erase(0, pos+delimiter.length());
        }

        line_machine_code += to_string(position_counter) + " ";

        if (instruction_table.count(tokens[0]) == 1) {
            line_machine_code += instruction_table[tokens[0]].first + " ";
            for (int i = 1; i < tokens.size(); i++) {
                line_machine_code += to_string(st[tokens[i]].first) + " ";
            }
            position_counter += instruction_table[tokens[0]].second;
        } else {
            if (directives_table.count(tokens[0]) == 1) {
                if (tokens[0] == "SPACE") {
                    line_machine_code += "00";
                } else {
                    line_machine_code += to_string(*symbol_table[position_counter]);
                }
                position_counter += directives_table[tokens[0]];
            } else {
                cout << "Erro! Operação não indentificada!\n";
            }
        }

        machine_code.push_back(line_machine_code);
        line_counter++;
    }

    return machine_code;
}

int main(int argc, char* argv[]) {
    string fileName = argv[1];

    ifstream inputFile(fileName);

    if (!inputFile.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << fileName << '\n';
        return 1;
    }
    
    vector<string> pp = pre_processing(inputFile);
    // map<string, pair<int, int*>> fp = first_pass(pp);
    // map<int, int*> symbol_table = transform_symbol_table(fp);
    // vector<string> sp = second_pass(pp, fp, symbol_table);

    for (auto l : pp) {
        cout << l << '\n';
    }

    inputFile.close();
    return 0;
}