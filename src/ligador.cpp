#include <bits/stdc++.h>

void linker(std::vector<std::string> obj_files) {
    std::vector<int> offsets = {0};
    std::map<std::string, int> global_definition_table;
    std::map<std::string, std::vector<int>> usage_table;
    std::vector<int> machine_code;
    int curr = 0;

    for (std::string& fn : obj_files) {
        std::ifstream obj_file(fn);
        std::string line;    
        std::vector<std::string> labels;    

        if (!obj_file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo: " << fn << '\n';
            return;
        }

        while (getline(obj_file, line)) {
            if (line.substr(0, 2) == "D,") {
                line = line.substr(3);
                
                size_t pos = line.find(" ");
                std::string label = line.substr(0, pos);
                
                line.erase(0, pos+1);
                std::string position = line.substr((pos = line.find(" "))+1);

                global_definition_table[label] = stoi(position);
                labels.push_back(label);
            } else if (line.substr(0, 2) == "U,") {
                continue;
            } else if (line.substr(0, 2) == "R,") {
                continue;
            } else {
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

        offsets.push_back(offsets[curr]+machine_code.size());

        for (std::string& l : labels) {
            global_definition_table[l] += offsets[curr];
        }

        obj_file.close();
        curr++;
    }

    curr = 0;

    for (std::string& fn : obj_files) {
        std::ifstream obj_file(fn);
        std::string line;    

        if (!obj_file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo: " << fn << '\n';
            return;
        }

        while (getline(obj_file, line)) {
            if (line.substr(0, 2) == "D,") {
                continue;
            } else if (line.substr(0, 2) == "U,") {
                line = line.substr(3);
                
                size_t pos = line.find(" ");
                std::string label = line.substr(0, pos);
                
                line.erase(0, pos+1);
                std::string position = line.substr((pos = line.find(" "))+1);

                machine_code[offsets[curr]+stoi(position)] += global_definition_table[label];
            } else if (line.substr(0, 2) == "R,") {
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

    std::ofstream executable_file("./" + obj_files[0].substr(0, obj_files[0].size()-4) + ".e");

    for (int i : machine_code) {
        executable_file << i << " ";
    }

    return;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> file_names;

    for (int i = 1; i < argc; i++) {
        file_names.push_back(argv[i]);
    }

    if (file_names.size() == 1) {
        std::cerr << "Erro: arquivo(s) inválido(s)!\n";
        return 1;
    } else {
        if (all_of(file_names.begin(), file_names.end(), [](std::string& s) {return s.substr(s.size()-4, 4) == ".obj";})) {
            linker(file_names);
        } else {
            std::cerr << "Erro: arquivo(s) inválido(s)!\n";
            return 1;
        }
    }
    return 0;
}