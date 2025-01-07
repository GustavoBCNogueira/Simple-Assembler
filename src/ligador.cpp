#include <bits/stdc++.h>

using namespace std;

void linker(vector<string> objFiles) {
    map<string, int> global_symbol_table; // Tabela global de símbolos
    vector<string> resolved_code;        // Código final resolvido
    int base_address = 0;                // Endereço base para realocação

    for (const auto& file : objFiles) {
        ifstream inputFile(file);
        if (!inputFile.is_open()) {
            cerr << "Erro ao abrir o arquivo: " << file << '\n';
            continue;
        }

        string line;
        while (getline(inputFile, line)) {
            if (line[0] == 'D') {
                // Definições
                string symbol;
                int value;
                sscanf(line.c_str(), "D, %s %d", &symbol, &value);
                global_symbol_table[symbol] = value + base_address;
            } else if (line[0] == 'U') {
                // Referências não resolvidas (ainda não tratadas)
            } else if (line[0] == 'R') {
                // Tabela de realocação
                // Lógica para aplicar as realocações ao código
            } else {
                // Código do programa
                resolved_code.push_back(line);
            }
        }

        // Atualiza endereço base
        base_address += resolved_code.size();
        inputFile.close();
    }

    // Imprime código final resolvido
    for (const auto& line : resolved_code) {
        cout << line << '\n';
    }
}

int main(int argc, char* argv[]) {
    vector<string> file_names;

    for (int i = 1; i < argc; i++) {
        file_names.push_back(argv[i]);
    }

    if (file_names.size() == 1) {
        cerr << "Erro: arquivo(s) inválido(s)!\n";
        return 1;
    } else {
        if (all_of(file_names.begin(), file_names.end(), [](string& s) {return s.substr(s.size()-4, 4) == ".obj";})) {
            linker(file_names);
        } else {
            cerr << "Erro: arquivo(s) inválido(s)!\n";
            return 1;
        }
    }
    return 0;
}