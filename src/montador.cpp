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
    // vamos definir a mnt assim????
    map<string, int> macro_name_table;
    vector<vector<string>> macro_definition_table;
    while (getline(file, line)) {
        trim(line);

        while ((pos = line.find(delimiter)) != line.npos) {
            line.erase(pos);
        }

        if (line.find("ENDMACRO") != line.npos) {
            macro_definition_table.push_back(macro_body);
            macro_body.clear();
            macro = false;
            continue;
        }

        if (macro) {
            macro_body.push_back(line);
            continue;
        }

        if (line.find("MACRO") != line.npos) {
            string macro_name = line.substr(0, line.find(':'));
            if (macro_name_table.count(macro_name) == 1) {
                cout << "Erro! Macro redefinida!\n";
            } else {
                macro_name_table.insert({macro_name, macro_definition_table.size()});
                macro = true;
            }
            continue;
        } 

        int i = 0;
        string macro_name = "";

        while (line[i] <= 90) {
            macro_name += line[i];
        }

        if (macro_name_table.count(macro_name) == 0) {
            cout << "Erro! Macro não declarada!\n";
        } else {
            // ainda sem tratar os parametros das macros
            for (auto l : macro_definition_table[macro_name_table[macro_name]]) {
                pre_processed_code.push_back(l);
            }
        }
    }
}

map<string, int> first_pass(ifstream& file) {
    int position_counter = 0, line_counter = 1;
    string line, delimiter = " ";
    map<string, int> symbol_table;
    map<string, pair<string, int>> instruction_table;
    map<string, int> directives_table = {{"CONST", 1}, {"SPACE", 1}};

    build_instruction_table(instruction_table);

    while (getline(file, line)) {
        vector<string> tokens;
        size_t pos = 0;
        string token;

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
        
        // ignorando os comentários
        for (int i = 0; i < tokens.size(); i++) {
            auto it = tokens[i].find(";");
            if (it != tokens[i].npos) {
                tokens.erase(tokens.begin()+i, tokens.end());
                break;
            }
        }

        if (tokens[0].find(':') != tokens[0].npos) {
            // caso que existe rótulo na line
            tokens[0].erase(tokens[0].end());
            if (symbol_table.count(tokens[0]) == 1) {
                cout << "Erro! Símbolo redefinido!\n";
            } else {
                symbol_table.insert({tokens[0], position_counter});
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
                position_counter += directives_table[operation];
            } else {
                cout << "Erro! Operação não identificada!\n";
            }
        }

        line_counter++;
    }

    return symbol_table;     // retornando a tabela de símbolos para a segunda passagem
}


void second_pass(ifstream& file, map<string, int>& symbol_table) {
    int position_counter = 0, line_counter = 1;
    string line, delimiter = " ";
    map<string, pair<string, int>> instruction_table;
    map<string, int> directives_table = {{"CONST", 1}, {"SPACE", 1}};
    vector<string> machine_code;

    build_instruction_table(instruction_table);

    while (getline(file, line)) {
        vector<string> tokens;
        size_t pos = 0;
        string token, line_machine_code = "";

        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);

            if (token.find(';') != token.npos) {
                break;
            }

            if (token.find(':') != token.npos) {
                tokens.push_back(token);
            }

            line.erase(0, pos+delimiter.length());
        }
        tokens.push_back(line); // temos que ver isso caso tiver comentarios


        // considerando que todos os operandos são símbolos
        for (int i = 1; i < tokens.size(); i++) {
            if (symbol_table.count(tokens[i]) == 0) {
                cout << "Erro! Símbolo redefinido!\n";
            }
        }

        if (instruction_table.count(tokens[0]) == 1) {
            // adiciona o endereço no código máquina ????
            line_machine_code += to_string(position_counter);

            line_machine_code += instruction_table[tokens[0]].first;
            for (int i = 1; i < tokens.size(); i++) {
                line_machine_code += to_string(symbol_table[tokens[i]]);
            }
            position_counter += instruction_table[tokens[0]].second;
        } else {
            if (directives_table.count(tokens[0]) == 1) {
                // adiciona o endereço no código máquina ????
                line_machine_code += to_string(position_counter);
                line_machine_code += "XX"; // se for CONST, coloca o valor da constante
                // chama a subrotina que executa a diretiva TODO
                position_counter += directives_table[tokens[0]];
            } else {
                cout << "Erro! Operação não indentificada!\n";
            }
        }

        machine_code.push_back(line_machine_code);
        line_counter++;
    }
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