#include <bits/stdc++.h>

// Função linker
// Argumentos:
//      - obj_files: vetor com os nomes dos arquivos objetos a serem ligados, 
//      do tipo vetor de strings
// Valor de retorno: void
// Objetivo: ligar os arquivos objetos, criando um só executável
void linker(std::vector<std::string> obj_files) {
    // offsets: vetor com os fatores de correção de cada módulo, na ordem em que estão
    // no obj_files
    std::vector<int> offsets = {0};
    
    // Tabela global de definições, com o nome do símbolo como chave e a sua posição no
    // código como valor
    std::map<std::string, int> global_definition_table;

    // machine_code: vetor de inteiros onde cada posição do vetor representa um endereço do
    // código executável
    std::vector<int> machine_code;
    int curr = 0;

    // Loop que passa por cada módulo com base no nome do seu arquivo, 
    // constrói a tabela de definições globais e alinha os módulos
    for (std::string& fn : obj_files) {

        // obj_file: arquivo objeto da iteração atual
        std::ifstream obj_file(fn);
        std::string line;    

        if (!obj_file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo: " << fn << '\n';
            return;
        }

        // Loop para analisar linha a linha do arquivo
        while (getline(obj_file, line)) {
            if (line.substr(0, 2) == "D,") {
                // Caso há uma definição de label pública na linha, 
                // coloca a mesma na tabela global de definições
                line = line.substr(3);
                
                size_t pos = line.find(" ");
                std::string label = line.substr(0, pos);
                
                line.erase(0, pos+1);
                std::string position = line.substr((pos = line.find(" "))+1);

                global_definition_table[label] = offsets[curr] + stoi(position);
            } else if (line.substr(0, 2) == "U,") {
                continue;
            } else if (line.substr(0, 2) == "R,") {
                continue;
            } else {
                // Caso for a linha do código de máquina,
                // alinha o código com o vetor machine_code 
                std::string address;
                size_t pos;
                while ((pos = line.find(" ")) != line.npos) {
                    address = line.substr(0, pos);
                    
                    if (address == " ") {
                        break;
                    } 

                    machine_code.push_back(stoi(address));
                    line.erase(0, pos+1);
                }
            }
        }

        // Atualiza o vetor de fatores de correção com o tamanho do código
        offsets.push_back(offsets[curr]+machine_code.size());

        obj_file.close();
        curr++;
    }

    // Zerando o contador para o novo loop
    curr = 0;

    // Loop que passa por cada módulo com base no nome do seu arquivo, 
    // corrige as posições de endereços externos e relativos
    for (std::string& fn : obj_files) {

        // obj_file: arquivo objeto da iteração atual
        std::ifstream obj_file(fn);
        std::string line;    

        if (!obj_file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo: " << fn << '\n';
            return;
        }

        // Loop para analisar cada linha do arquivo
        while (getline(obj_file, line)) {
            if (line.substr(0, 2) == "D,") {
                continue;
            } else if (line.substr(0, 2) == "U,") {
                // Caso seja um símbolo externo, corrige o endereço do mesmo
                // no código com base na tabela geral de definições
                line = line.substr(3);
                
                size_t pos = line.find(" ");
                std::string label = line.substr(0, pos);
                
                line.erase(0, pos+1);
                std::string position = line.substr((pos = line.find(" "))+1);

                machine_code[offsets[curr]+stoi(position)] += global_definition_table[label];
            } else if (line.substr(0, 2) == "R,") {
                // Caso seja um endereço relativo, corrige o mesmo
                // no código com base no vetor de fatores de correção
                line = line.substr(3);
    
                int bit_map_index = 0;
                size_t pos;
                std::string bit;

                while ((pos = line.find(" ")) != line.npos) {
                    bit = line.substr(0, pos);
                    
                    if (bit == " ") {
                        break;
                    } 

                    if (bit == "1") {
                        machine_code[offsets[curr]+bit_map_index] += offsets[curr];
                    }

                    bit_map_index++;
                    line.erase(0, pos+1);
                }

            } else {
                continue;
            }
        }

        obj_file.close();
        curr++;
    }

    // Criando e escrevendo no arquivo executável 
    std::ofstream executable_file("./" + obj_files[0].substr(0, obj_files[0].size()-4) + ".e");

    for (int i : machine_code) {
        executable_file << i << " ";
    }

    return;
}

int main(int argc, char* argv[]) {
    // Vetor com os nomes dos arquivos objeto
    std::vector<std::string> file_names;

    for (int i = 1; i < argc; i++) {
        file_names.push_back(argv[i]);
    }

    if (file_names.size() != 2) {
        std::cerr << "Erro: arquivo(s) inválido(s)!\n";
        return 1;
    } else {
        // Caso todos os arquivos forem do tipo .obj, ocorre a ligação
        if (all_of(file_names.begin(), file_names.end(), [](std::string& s) {return s.substr(s.size()-4, 4) == ".obj";})) {
            linker(file_names);
        } else {
            std::cerr << "Erro: arquivo(s) inválido(s)!\n";
            return 1;
        }
    }
    
    return 0;
}