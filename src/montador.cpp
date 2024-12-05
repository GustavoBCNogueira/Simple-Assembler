#include <bits/stdc++.h>

using namespace std;

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

void build_tabela_instrucoes(map<string, pair<string, int>>& tabela_instrucoes) {
    // primeiro elemento do pair: opcode
    // segundo elemento do pair: número de operandos
    tabela_instrucoes = {{"add", {"01", 2}}, {"sub", {"02", 2}}, {"mul", {"03", 2}},
                         {"div", {"04", 2}}, {"jmp", {"05", 2}}, {"jmpn", {"06", 2}}, 
                         {"jmpp", {"07", 2}}, {"jmpz", {"08", 2}}, {"copy", {"09", 3}}, 
                         {"load", {"10", 2}}, {"store", {"11", 2}}, {"input", {"12", 2}}, 
                         {"output", {"13", 2}}, {"stop", {"14", 1}}};
    return;
}

void pre_processamento(ifstream& file) {
    string linha;
    bool macro = false;
    vector<string> macro_body;
    // vamos definir a mnt assim????
    map<string, int> macro_name_table;
    vector<vector<string>> macro_definition_table;
    while (getline(file, linha)) {
        trim(linha);
        if (macro) {
            macro_body.push_back(linha);
            continue;
        }

        if (linha.find("ENDMACRO") != linha.npos) {
            macro_definition_table.push_back(macro_body);
            macro_body.clear();
            macro = false;
        }

        if (linha.find("MACRO") != linha.npos) {
            string macro_name = linha.substr(0, linha.find(':'));
            if (macro_name_table.count(macro_name) == 1) {
                cout << "Erro! Macro redefinida!\n";
            } else {
                macro_name_table.insert({macro_name, macro_definition_table.size()});
                macro = true;
            }
        }
    }
}

map<string, int> primeira_passagem(ifstream& file) {
    int contador_posicao = 0, contador_linha = 1;
    string linha, delimiter = " ";
    map<string, int> tabela_simbolos;
    map<string, pair<string, int>> tabela_instrucoes;
    map<string, int> tabela_diretivas = {{"CONST", 1}, {"SPACE", 1}};

    build_tabela_instrucoes(tabela_instrucoes);

    while (getline(file, linha)) {
        vector<string> tokens;
        size_t pos = 0;
        string token;

        while ((pos = linha.find(delimiter)) != linha.npos) {
            token = linha.substr(0, pos);
            tokens.push_back(token);
            linha.erase(0, pos+delimiter.length());
        }
        tokens.push_back(linha);
        
        // ignorando os comentários
        for (int i = 0; i < tokens.size(); i++) {
            auto it = tokens[i].find(";");
            if (it != tokens[i].npos) {
                tokens.erase(tokens.begin()+i, tokens.end());
                break;
            }
        }

        if (tokens[0].find(':') != tokens[0].npos) {
            // caso que existe rótulo na linha
            tokens[0].erase(tokens[0].end());
            if (tabela_simbolos.count(tokens[0]) == 1) {
                cout << "Erro! Símbolo redefinido!\n";
            } else {
                tabela_simbolos.insert({tokens[0], contador_posicao});
            }
            tokens.erase(tokens.begin());
        }

        string operacao = tokens[0];
        
        if (tabela_instrucoes.count(operacao) == 1) {
            // verificando apenas se a quantidade de operandos está correta, 
            // falta verificar a corretude (2a passagem)
            if (tokens.size()-1 != tabela_instrucoes[tokens[0]].second) { 
                cout << "Erro! Operandos inválidos!\n";
            }
            contador_posicao += tabela_instrucoes[operacao].second;
        } else {
            if (tabela_diretivas.count(operacao) == 1) {
                // chama subrotina que executa a tarefa TODO
                contador_posicao += tabela_diretivas[operacao];
            } else {
                cout << "Erro! Operação não identificada!\n";
            }
        }

        contador_linha++;
    }

    return tabela_simbolos;     // retornando a tabela de símbolos para a segunda passagem
}


void segunda_passagem(ifstream& file, map<string, int>& tabela_simbolos) {
    int contador_posicao = 0, contador_linha = 1;
    string linha, delimiter = " ";
    map<string, pair<string, int>> tabela_instrucoes;
    map<string, int> tabela_diretivas = {{"CONST", 1}, {"SPACE", 1}};
    vector<string> codigo_maquina;

    build_tabela_instrucoes(tabela_instrucoes);

    while (getline(file, linha)) {
        vector<string> tokens;
        size_t pos = 0;
        string token, linha_codigo_maquina = "";

        while ((pos = linha.find(delimiter)) != linha.npos) {
            token = linha.substr(0, pos);

            if (token.find(';') != token.npos) {
                break;
            }

            if (token.find(':') != token.npos) {
                tokens.push_back(token);
            }

            linha.erase(0, pos+delimiter.length());
        }
        tokens.push_back(linha); // temos que ver isso caso tiver comentarios


        // considerando que todos os operandos são símbolos
        for (int i = 1; i < tokens.size(); i++) {
            if (tabela_simbolos.count(tokens[i]) == 0) {
                cout << "Erro! Símbolo redefinido!\n";
            }
        }

        if (tabela_instrucoes.count(tokens[0]) == 1) {
            // adiciona o endereço no código máquina ????
            linha_codigo_maquina += to_string(contador_posicao);

            linha_codigo_maquina += tabela_instrucoes[tokens[0]].first;
            for (int i = 1; i < tokens.size(); i++) {
                linha_codigo_maquina += to_string(tabela_simbolos[tokens[i]]);
            }
            contador_posicao += tabela_instrucoes[tokens[0]].second;
        } else {
            if (tabela_diretivas.count(tokens[0]) == 1) {
                // adiciona o endereço no código máquina ????
                linha_codigo_maquina += to_string(contador_posicao);
                linha_codigo_maquina += "XX"; // se for CONST, coloca o valor da constante
                // chama a subrotina que executa a diretiva TODO
                contador_posicao += tabela_diretivas[tokens[0]];
            } else {
                cout << "Erro! Operação não indentificada!\n";
            }
        }

        codigo_maquina.push_back(linha_codigo_maquina);
        contador_linha++;
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