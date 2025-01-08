#include <bits/stdc++.h>

using namespace std;

void linker(vector<string> objFiles) {
    vector<int> offsets = {0};
    map<string, int> global_definition_table;
    map<string, vector<int>> usage_table;
    int curr = 0;

    for (string& fn : objFiles) {
        ifstream obj_file(fn);
        string line, machine_code;    
        vector<string> labels;    

        if (!obj_file.is_open()) {
            cerr << "Erro ao abrir o arquivo: " << fn << '\n';
            return;
        }

        // obj_file.seekg(-1, ios::end);

        // char ch;
        // while (true) {
        //     obj_file.get(ch);

        //     if ((int) obj_file.tellg() <= 1) {
        //         obj_file.seekg(0);
        //         break;
        //     } else if (ch == '\n') {
        //         break;
        //     } else {
        //         obj_file.seekg(-2, ios::cur);
        //     }
        // }

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
                machine_code = line;
            }
        }
        
        size_t pos;
        string token;
        int count = 0;
        while ((pos = machine_code.find(" ")) != machine_code.npos) {
            token = machine_code.substr(0, pos);
            
            if (token == " ") {
                break;
            } 

            count++;
            machine_code.erase(0, pos+1);
        }

        offsets.push_back(offsets[curr]+count);

        for (string& l : labels) {
            global_definition_table[l] += offsets[curr];
        }

        obj_file.close();
        curr++;
    }


}

int main(int argc, char* argv[]) {
    vector<string> file_names;

    for (int i = 1; i < argc; i++) {
        file_names.push_back(argv[i]);
    }

    // if (file_names.size() == 1) {
    //     cerr << "Erro: arquivo(s) inválido(s)!\n";
    //     return 1;
    // } else {
        if (all_of(file_names.begin(), file_names.end(), [](string& s) {return s.substr(s.size()-4, 4) == ".obj";})) {
            linker(file_names);
        } else {
            cerr << "Erro: arquivo(s) inválido(s)!\n";
            return 1;
        }
    // }
    return 0;
}