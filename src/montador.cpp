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
    instruction_table = {{"add", {"01", 2}}, {"sub", {"02", 2}}, {"mul", {"03", 2}},
                         {"div", {"04", 2}}, {"jmp", {"05", 2}}, {"jmpn", {"06", 2}}, 
                         {"jmpp", {"07", 2}}, {"jmpz", {"08", 2}}, {"copy", {"09", 3}}, 
                         {"load", {"10", 2}}, {"store", {"11", 2}}, {"input", {"12", 2}}, 
                         {"output", {"13", 2}}, {"stop", {"14", 1}}};
    return;
}

vector<string> pre_processing(ifstream& file) {
    string line, delimiter = " ";
    bool macro = false;
    size_t pos;
    vector<string> macro_body, pre_processed_code;
    map<string, int> macro_name_table;
    vector<vector<string>> macro_definition_table;

    while (getline(file, line)) {
        string token;
        vector<string> tokens;
        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);
            trim(token);

            // descartando os comentários
            if (token.find(";") != token.npos) {
                break;
            }

            if (token.size()) {
                tokens.push_back(token);
            }
            line.erase(0, pos+delimiter.length());
        }

        if (line.find(";") == line.npos) {
            tokens.push_back(line);
        }

        if (tokens[0] == "ENDMACRO") {
            macro_definition_table.push_back(macro_body);
            macro_body.clear();
            macro = false;
            continue;
        }

        if (macro) {
            string temp = "";
            for (auto t : tokens) {
                temp += t+" ";
            }
            macro_body.push_back(temp+'\n');
            continue;
        }

        if (tokens[1].find("MACRO") != tokens[1].npos) {
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
            for (auto l : macro_definition_table[macro_name_table[tokens[0]]]) {
                pre_processed_code.push_back(l);
            }
        } else {
            // código reusado
            string temp = "";
            
            for (auto t : tokens) {
                temp += t+" ";
            }
            
            pre_processed_code.push_back(temp+'\n');
        }
    }

    return pre_processed_code;
}

map<string, pair<int, int*>> first_pass(vector<string> pre_processed_code) {
    int position_counter = 0, line_counter = 1;
    string delimiter = " ";
    map<string, pair<int, int*>> symbol_table;
    map<string, pair<string, int>> instruction_table;
    map<string, int> directives_table = {{"CONST", 1}, {"SPACE", 1}};

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
            tokens.push_back(token);
            line.erase(0, pos+delimiter.length());
        }
        tokens.push_back(line);

        if (tokens[0].find(':') != tokens[0].npos) {
            // caso que existe rótulo na linha
            tokens[0].erase(tokens[0].end());
            label = tokens[0];
            if (symbol_table.count(label) == 1) {
                cout << "Erro! Símbolo redefinido!\n";
            } else {
                symbol_table.insert({label, {position_counter, (int*) malloc(4)}});
            }
            tokens.erase(tokens.begin());
        }

        string operation = tokens[0];
        
        if (instruction_table.count(operation) == 1) {
            // verificando apenas se a quantidade de operandos está correta, 
            // falta verificar a corretude (2a passagem)
            if (tokens.size()-1 != instruction_table[tokens[0]].second) { 
                cout << "Erro! Operandos inválidos!\n";
            }
            position_counter += instruction_table[operation].second;
        } else {
            if (directives_table.count(operation) == 1) {
                // chama subrotina que executa a tarefa TODO
                if (operation == "CONST") {
                    *symbol_table[label].second = stoi(tokens[1]);                   
                }
                position_counter += directives_table[operation];
            } else {
                cout << "Erro! Operação não identificada!\n";
            }
        }

        line_counter++;
    }

    return symbol_table;     // retornando a tabela de símbolos para a segunda passagem
}


vector<string> second_pass(vector<string> pre_processed_code, map<int, int*>& symbol_table) {
    int position_counter = 0, line_counter = 1;
    string delimiter = " ";
    map<string, pair<string, int>> instruction_table;
    map<string, int> directives_table = {{"CONST", 1}, {"SPACE", 1}};
    vector<string> machine_code;

    build_instruction_table(instruction_table);

    for (string line : pre_processed_code) {
        vector<string> tokens;
        size_t pos = 0;
        string token, line_machine_code = "";

        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);

            if (token.find(':') != token.npos) {
                continue;
            }

            line.erase(0, pos+delimiter.length());
        }
        tokens.push_back(line);


        // // considerando que todos os operandos são símbolos
        // for (int i = 1; i < tokens.size(); i++) {
        //     if (symbol_table.count(tokens[i]) == 0) {
        //         cout << "Erro! Símbolo redefinido!\n";
        //     }
        // }

        line_machine_code += to_string(position_counter);

        if (instruction_table.count(tokens[0]) == 1) {

            line_machine_code += instruction_table[tokens[0]].first;
            for (int i = 1; i < tokens.size(); i++) {
                line_machine_code += tokens[i];
            }
            position_counter += instruction_table[tokens[0]].second;
        } else {
            if (directives_table.count(tokens[0]) == 1) {
                if (tokens[0] == "SPACE") {
                    line_machine_code += "XX";
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

int main() {
    string fileName;
    cin >> fileName;

    if (fileName.empty()) 
        cout << "Erro! " << fileName << "não tem nada!\n";
    else {
        ifstream inputStream(fileName);
    }


    return 0;
}