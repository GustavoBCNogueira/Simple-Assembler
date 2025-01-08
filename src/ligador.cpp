#include <bits/stdc++.h>

using namespace std;

void linker(vector<string> obj_files) {
    vector<int> offsets = {0};
    map<string, int> global_definition_table;
    map<string, vector<int>> usage_table;
    vector<int> machine_code;
    int curr = 0;

    for (string& fn : obj_files) {
        ifstream obj_file(fn);
        string line;    
        vector<string> labels;    

        if (!obj_file.is_open()) {
            cerr << "Erro ao abrir o arquivo: " << fn << '\n';
            return;
        }

        while (getline(obj_file, line)) {
            if (line.substr(0, 2) == "D,") {
                line = line.substr(3);
                
                size_t pos = line.find(" ");
                string label = line.substr(0, pos);
                
                line.erase(0, pos+1);
                string position = line.substr((pos = line.find(" "))+1);

                global_definition_table[label] = stoi(position);
                labels.push_back(label);
            } else if (line.substr(0, 2) == "U,") {
                continue;
            } else if (line.substr(0, 2) == "R,") {
                continue;
            } else {
                string address;
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

        for (string& l : labels) {
            global_definition_table[l] += offsets[curr];
        }

        obj_file.close();
        curr++;
    }

    curr = 0;

    for (string& fn : obj_files) {
        ifstream obj_file(fn);
        string line;    

        if (!obj_file.is_open()) {
            cerr << "Erro ao abrir o arquivo: " << fn << '\n';
            return;
        }

        while (getline(obj_file, line)) {
            if (line.substr(0, 2) == "D,") {
                continue;
            } else if (line.substr(0, 2) == "U,") {
                line = line.substr(3);
                
                size_t pos = line.find(" ");
                string label = line.substr(0, pos);
                
                line.erase(0, pos+1);
                string position = line.substr((pos = line.find(" "))+1);

                machine_code[offsets[curr]+stoi(position)] += global_definition_table[label];
            } else if (line.substr(0, 2) == "R,") {
                line = line.substr(3);
    
                int bit_map_index = 0;
                size_t pos;
                string bit;

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

    for (int i : machine_code) {
        cout << i << " ";
    }
    cout << '\n';

    ofstream executable_file("./" + obj_files[0].substr(0, obj_files[0].size()-4) + ".e");

    for (int i : machine_code) {
        executable_file << i << " ";
    }

    return;
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