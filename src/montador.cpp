#include <bits/stdc++.h>

using namespace std;

void readFile(ifstream& file) {
    string line;
    while (getline(file, line)) {
        cout << line << '\n';
    }
}

void build_tabela_instrucoes(map<string, int>& tabela_instrucoes) {
    tabela_instrucoes = {{"add", 2}, {"sub", 2}, {"mul", 2},
                         {"div", 2}, {"jmp", 2}, {"jmpn", 2}, 
                         {"jmpp", 2}, {"jmpz", 2}, {"copy", 3}, 
                         {"load", 2}, {"store", 2}, {"input", 2}, 
                         {"output", 2}, {"stop", 1}};
}

void primeira_passagem(ifstream& file) {
    int contador_posicao = 0;
    int contador_linha = 1;
    string linha, delimiter = " ";
    map<string, int> tabela_simbolos;
    map<string, int> tabela_instrucoes;
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
        // ignorar os comentários

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
            contador_posicao += tabela_instrucoes[operacao];
        } else {
            if (tabela_diretivas.count(operacao) == 1) {
                // chama subrotina que executa a tarefa
                // contador_posicao += retorno da subrotina
            } else {
                cout << "Erro! Operação não identificada!\n";
            }
        }

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

        if (!inputStream) 
            cout << "Erro! " << fileName << "é um arquivo inválido!\n";
        else {
            readFile(inputStream);
        }

    }


    return 0;
}