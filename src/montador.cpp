#include <bits/stdc++.h>

// Variável global para indicar se o programa terá que ser ligado (true) ou não (false)
bool has_link = false;

// Função para retirar os espaços à esquerda da string
inline void ltrim(std::string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
}


// Função para retirar os espaços à direita da string
inline void rtrim(std::string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());
}


// Função para retirar os espaços à esquerda e à direita da string
inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}


std::map<int, int*> transform_symbol_table(std::map<std::string, std::pair<std::pair<int, int*>, bool>>& symbol_table) {
    std::map<int, int*> m;
    for (auto s : symbol_table) 
        m.insert({s.second.first.first, s.second.first.second});

    return m;
}


// Função pre_processing
// Argumentos:
//      - file_name: nome do arquivo .asm que será pré-processado, do tipo string;
//      - file: arquivo .asm que será pré-processado, do tipo ifstream.
// Valor de retorno: void
// Objetivo: gerar o arquivo com o código pré-processado
void pre_processing(std::string& file_name, std::ifstream& file) {
    std::string line, delimiter = " ";
    bool macro = false, text = false;

    // pre_processed_code, text_code e data_code:
    //      - vetores de string que armazenam as linhas do código pré-processado;
    //      - text_code e data_code correspondem às seções de texto e dados do programa;
    //      - as duas seções são organizadas da maneira especificada no trabalho e 
    //        armazenadas em pre_processed_code.
    std::vector<std::string> pre_processed_code, text_code, data_code;

    // macro_name_table: armazena os nomes das macros e o seu respectivo índice na MDT
    // macro_definition_table: armazena os escopos das macros
    std::map<std::string, int> macro_name_table;
    std::vector<std::string> macro_definition_table;

    // Loop para analisar o arquivo .asm linha a linha
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        
        // Transformando toda a linha para maiúsculo (tratando a não sensibilidade ao caso)
        transform(line.begin(), line.end(), line.begin(), ::toupper);

        if (line.find("SECTION TEXT") != line.npos) { 
            text = true;
            continue;
        } 
        
        if (line.find("SECTION DATA") != line.npos) {
            text = false;
            continue;
        }

        line += " ";


        // Definição de variáveis para a busca de tokens e o seu tratamento
        // token: token atual em análise
        // tokens: conjunto de tokens da linha atual
        // has_pointer_aritmetic: indica se há aritmética de ponteiros na linha
        // comment: indica se há comentário na linha

        std::string token;
        std::vector<std::string> tokens;
        size_t pos, comma;
        bool has_pointer_arithmetic = false, comment = false;

        // Loop para analisar os tokens
        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);
            trim(token);

            if (token.size() == 0) {
                line.erase(0, pos+delimiter.length());
                continue;
            }

            // Descartando os comentários
            if (token.find(";") != token.npos) {
                token = token.substr(0, token.find(";"));
                comment = true;
            }

            if (has_pointer_arithmetic) {
                // Caso em que a expressão de aritmética de ponteiros ainda não foi concluída 
                // no token anterior
                tokens[tokens.size()-1] += token;
                has_pointer_arithmetic = false;
            } else if (token.find("+") != token.npos) {
                // Se há um caracter "+" no token, é uma operação de aritmética de ponteiros
                if (token == "+") {
                    tokens[tokens.size()-1] += token;
                    has_pointer_arithmetic = true;
                } else if (token.substr(0, token.find("+")).size() > 0) {
                    if (token.substr(token.find("+")+1).size() == 0) {
                        has_pointer_arithmetic = true;
                    }
                    tokens.push_back(token);
                } else {
                    tokens[tokens.size()-1] += token;
                }
            } else {
                if (token.find(",") != token.npos) {
                    // Se há um caracter "," no token, é necessário tratar os operandos de 
                    // outra forma 
                    std::string t;
                    while ((comma = token.find(",")) != token.npos) {
                        t = token.substr(0, comma);
                        trim(t);
                    
                        if (t.size() == 0) {
                            token.erase(0, comma+1);
                            continue;
                        }

                        tokens.push_back(t);
                        token.erase(0, comma+1);
                    }
                }

                if (token.size()) {
                    tokens.push_back(token);
                }
            }

            if (comment) {
                break;
            }

            line.erase(0, pos+delimiter.length());
        }

        if (macro) {
            // Caso em que a linha atual está dentro do escopo de uma macro
            std::string temp = "";

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
            // Declaração de macro
            std::string macro_name = tokens[0].substr(0, tokens[0].find(":"));
            trim(macro_name);

            if (macro_name_table.count(macro_name) == 1) {
                std::cerr << "Erro! Macro redefinida!\n";
            } else {
                macro_name_table.insert({macro_name, macro_definition_table.size()});
                macro = true;
            }
           
            continue;
        } 

        if (macro_name_table.count(tokens[0]) == 1) {
            // Chamada de macro na seção de texto do código
            int idx = macro_name_table[tokens[0]];
           
            while (macro_definition_table[idx].find("ENDMACRO") == macro_definition_table[idx].npos) {
                text_code.push_back(macro_definition_table[idx]);
                idx++;
            }
        } else {
            // Adicionando a linha na sua seção correspondente
            std::string temp = "";
            
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

    // Reorganização do código, posicionando a seção de dados no fim do mesmo
    for (std::string l : text_code) 
        pre_processed_code.push_back(l);
    
    for (std::string l : data_code) 
        pre_processed_code.push_back(l);
    
    // Criando e escrevendo no arquivo pré-processado
    std::string pre_processed_file_name = file_name.substr(0, file_name.size()-4) + ".pre";
    std::ofstream pre_file("./" + pre_processed_file_name);
    std::ostream_iterator<std::string> pre_processed_file_iterator(pre_file, "\n");

    if (pre_file.is_open()) {
        copy(begin(pre_processed_code), end(pre_processed_code), pre_processed_file_iterator);
    }

    pre_file.close();

    return;
}

// Função first_pass
// Argumento: 
//      - pre_processed_code: arquivo do código pré processado, saída da função de pré-processamento, 
//        do tipo ifstream
// Valor de retorno: tupla com a Tabela de Símbolos, a tabela symbol_values e a Tabela de Definições, respectivamente
// Objetivo: construir a Tabela de Símbolos e a Tabela de Definições, caso necessário
std::tuple<std::map<std::string, std::pair<int, bool>>, std::map<int, int*>, std::map<std::string, int>> first_pass(std::ifstream& pre_processed_code) {
    
    // Contadores de posição e de linha
    int position_counter = 0, line_counter = 1;
    
    std::string delimiter = " ", line;
    
    // Tabela de Símbolos: map com o nome do símbolo como chave e um par indicando 
    // a posição da sua declaração no código e se é um símbolo externo ou não como valor
    std::map<std::string, std::pair<int, bool>> symbol_table;
    
    // symbol_values: tabela com as posições dos símbolos como chave e o seu conteúdo em memória
    // como valor
    std::map<int, int*> symbol_values;
    
    // Tabela de Definições: map com o nome do símbolo público como chave e 
    // a sua respectiva posição no código como valor 
    std::map<std::string, int> definitions_table;

    std::map<std::string, std::pair<std::string, int>> instruction_table;

    // Tabela de Diretivas: contém o nome da diretiva como chave e 
    // o quanto de espaço que ela ocupa no código como valor (SPACE é variável)
    std::map<std::string, int> directives_table = {{"CONST", 1}, {"SPACE", 1}};

    // Primeiro elemento do par de valores da instrução: opcode
    // Segundo elemento do par de valores da instrução: número de operandos
    instruction_table = {{"ADD", {"1", 2}}, {"SUB", {"2", 2}}, {"MUL", {"3", 2}},
                         {"MULT", {"3", 2}}, {"DIV", {"4", 2}}, {"JMP", {"5", 2}}, 
                         {"JMPN", {"6", 2}}, {"JMPP", {"7", 2}}, {"JMPZ", {"8", 2}}, 
                         {"COPY", {"9", 3}}, {"LOAD", {"10", 2}}, {"STORE", {"11", 2}}, 
                         {"INPUT", {"12", 2}}, {"OUTPUT", {"13", 2}}, {"STOP", {"14", 1}}};

    // Loop para analisar o arquivo .pre linha a linha
    while (getline(pre_processed_code, line)) {

        // tokens: conjunto de tokens da linha
        // token: token atual em análise
        // label: nome da label declarada na linha, caso exista
        std::vector<std::string> tokens;
        size_t pos = 0;
        std::string token, label;

        // Loop para coletar os tokens da linha
        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);
            tokens.push_back(token);
            line.erase(0, pos+delimiter.length());
        }

        // Se é uma declaração de rótulo público, insere o mesmo na tabela de definições,
        // com 0 absoluto como posição
        if (tokens[0] == "PUBLIC") {
            definitions_table.insert({tokens[1], 0});
            continue;
        }


        // Se é um rótulo, verifica a sua corretude e insere o mesmo na tabela de símbolos
        if (tokens[0].find(':') != tokens[0].npos) {
            tokens[0].erase(tokens[0].find(':'));
            label = tokens[0];

            if (isdigit(label[0])) {
                std::cerr << "Erro na linha " << line_counter << ": erro léxico na criação do rótulo!\n";
                break;
            } else {
                bool error = false;
                for (auto c : label) {
                    if (!(isalnum(c) || c == '_')) {
                        std::cerr << "Erro na linha " << line_counter << ": erro léxico na criação do rótulo!\n";
                        error = true;
                    }
                }

                if (error) {
                    break;
                } 
            }

            if (symbol_table.count(label) == 1) {
                std::cerr << "Erro na linha " << line_counter << ": rótulo redefinido!\n";
                break;
            } else {
                symbol_table.insert({label, {position_counter, 0}});
            }
            
            tokens.erase(tokens.begin());
        }

        if (tokens.size() == 0) {
            continue;
        }

        std::string operation = tokens[0];

        // Se ainda há um rótulo na linha, detecta o erro
        if (operation.find(":") != operation.npos) {
            std::cerr << "Erro na linha " << line_counter << ": rótulo dobrado na mesma linha!\n";
            break;
        }

        // Se a operação for BEGIN, indica que o programa será ligado
        if (operation == "BEGIN") {
            has_link = true;
            continue;
        }

        if (operation == "END") {
            continue;
        }

        // Se a operação for EXTERN, altera a propriedade na tabela de símbolos
        if (operation == "EXTERN") {
            symbol_table[label].second = true;
            continue;
        }   
        
        if (instruction_table.count(operation) == 1) {
            // Caso em que uma operação é identificada na linha
            if (tokens.size() != instruction_table[operation].second) { 
                std::cerr << "Erro na linha " << line_counter << ": número de operandos errados para a instrução!\n";
                break;
            }
            position_counter += instruction_table[operation].second;
        } else {
            // Caso em que uma diretiva é identificada na linha
            if (directives_table.count(operation) == 1) {
                if (tokens.size() > 2) {
                    std::cerr << "Erro na linha " << line_counter << ": número de operandos errados para a diretiva!\n";
                    break;
                }

                if (operation == "CONST") {
                    if (tokens[1].find("0x") != tokens[1].npos) {
                        int num = 0;
                        for (int i = tokens[1].size()-1; tokens[1][i] != 'x'; i--) {
                            num += (int)pow(16, tokens[1].size()-1-i) * stoi(std::to_string(tokens[1][i]));
                        }
                        symbol_values.insert({symbol_table[label].first, &num});
                    } else{ 
                        symbol_values.insert({symbol_table[label].first, (int*) malloc(4)});
                        *symbol_values[symbol_table[label].first] = stoi(tokens[1]);                   
                    }
                    position_counter += directives_table[operation];
                } else {
                    if (tokens.size() == 2) {
                        symbol_values.insert({symbol_table[label].first, (int*) malloc(4*stoi(tokens[1]))});
                        position_counter += stoi(tokens[1]);
                    } else {
                        symbol_values.insert({symbol_table[label].first, (int*) malloc(4)});
                        position_counter += directives_table[operation];
                    }
                }
            } else {
                // Erro caso a operação é inválida
                std::cerr << "Erro na linha " << line_counter << ": operação não identificada!\n";
                break;
            }
        }

        line_counter++;
    }

    // Corrigindo os valores da Tabela de Definições
    for (auto d : definitions_table) {
        definitions_table[d.first] = symbol_table[d.first].first;
    }

    return {symbol_table, symbol_values, definitions_table};
}


// Função second_pass
// Argumentos:
//      - file_name: nome do arquivo .pre que será compilado, do tipo string;
//      - pre_processed_code: arquivo .pre que será compilado, do tipo ifstream; 
//      - symbol_table: tabela de símbolos conforme definida e retornada na primeira passagem;
//      - symbol_values: tabela de símbolos com o endereço do símbolo como chave e o seu conteúdo como valor;
//      - definitions_table: tabela de definições conforme definida e retornada na primeira passagem;
// Valor de retorno: tabela de uso, com o nome do símbolo como chave e as posições que o mesmo se
// encontra no código
// Objetivo: gerar o arquivo .obj
std::map<std::string, std::vector<int>> second_pass(std::string& file_name, std::ifstream& pre_processed_code, std::map<std::string, std::pair<int, bool>>& symbol_table, std::map<int, int*>& symbol_values, std::map<std::string, int>& definitions_table) {
    // Contadores de posição e de linha
    int position_counter = 0, line_counter = 1;
    
    // machine_code: armazena o código de máquina montado, 
    // com informações para ser ligado caso necessário
    // bit_map: mapa de bits indicando as posições relativas do código
    std::string delimiter = " ", machine_code = "", line, bit_map = "";
    std::map<std::string, std::pair<std::string, int>> instruction_table;

    // usage_table: tabela de uso, com o nome do símbolo como chave e as posições que o mesmo se
    // encontra no código
    std::map<std::string, std::vector<int>> usage_table;
    std::map<std::string, int> directives_table = {{"CONST", 1}, {"SPACE", 1}};

    // primeiro elemento do par de valores da instrução: opcode
    // segundo elemento do par de valores da instrução: número de operandos
    instruction_table = {{"ADD", {"1", 2}}, {"SUB", {"2", 2}}, {"MUL", {"3", 2}},
                         {"MULT", {"3", 2}}, {"DIV", {"4", 2}}, {"JMP", {"5", 2}}, 
                         {"JMPN", {"6", 2}}, {"JMPP", {"7", 2}}, {"JMPZ", {"8", 2}}, 
                         {"COPY", {"9", 3}}, {"LOAD", {"10", 2}}, {"STORE", {"11", 2}}, 
                         {"INPUT", {"12", 2}}, {"OUTPUT", {"13", 2}}, {"STOP", {"14", 1}}};

    // Loop para analisar o arquivo .pre linha a linha
    while (getline(pre_processed_code, line)) {
        // tokens: tokens coletados da linha
        // token: token em análise 
        std::vector<std::string> tokens;
        size_t pos = 0;
        std::string token;

        // Ignora partes do código que não constarão no código de máquina
        if (has_link && (line.find("PUBLIC") != line.npos || line.find("EXTERN") != line.npos)) {
            continue;
        }

        // Loop para analisar e coletar os tokens
        while ((pos = line.find(delimiter)) != line.npos) {
            token = line.substr(0, pos);

            if (token.find(",") != token.npos) {
                token.erase(token.find(","));
            }

            if (token.find(':') == token.npos) {
                tokens.push_back(token);
            } else {
                token.erase(token.size()-1);
            }

            line.erase(0, pos+delimiter.length());
        }

        std::string operation = tokens[0];

        if (instruction_table.count(operation) == 1) {
            // Analisando a instrução
            machine_code += instruction_table[tokens[0]].first + " ";
            bit_map += "0 ";

            for (int i = 1; i < tokens.size(); i++) {
                size_t plus = tokens[i].find("+"); 

                if (plus != tokens[i].npos && symbol_table.count(tokens[i].substr(0, plus)) == 1) {
                    std::string label = tokens[i].substr(0, plus);
                    machine_code += std::to_string(symbol_table[label].first + stoi(tokens[i].substr(plus+1))) + " ";

                    if (has_link && symbol_table[label].second) {
                        bit_map += "0 ";
                        if (usage_table.count(label) == 0) {
                            usage_table.insert({label, {position_counter+i}});
                        } else {
                            usage_table[label].push_back(position_counter+i);
                        }
                    } else {
                        bit_map += "1 ";
                    } 
                } else if (symbol_table.count(tokens[i]) == 1) {
                    std::string label = tokens[i];
                    machine_code += std::to_string(symbol_table[label].first) + " ";

                    if (has_link && symbol_table[label].second) {
                        bit_map += "0 ";
                        if (usage_table.count(label) == 0) {
                            usage_table.insert({label, {position_counter+i}});
                        } else {
                            usage_table[label].push_back(position_counter+i);
                        }
                    } else {
                        bit_map += "1 ";
                    }
                } else {
                    std::cerr << "Erro na linha " << line_counter << ": rótulo ausente!\n";
                }
            }
            position_counter += instruction_table[operation].second;
        } else {
            if (directives_table.count(operation) == 1) {
                // Analisando a diretiva
                if (tokens[0] == "SPACE") {
                    if (tokens.size() > 1) {
                        for (int i = 0; i < stoi(tokens[1]); i++) {
                            machine_code += "0 ";
                            bit_map += "0 ";
                        }
                    } else {
                        machine_code += "0 ";
                        bit_map += "0 ";
                    }
                } else {
                    bit_map += "0 ";
                    machine_code += std::to_string(*symbol_values[position_counter]) + " ";
                }
                position_counter += directives_table[tokens[0]];
            } else {
                if (operation == "BEGIN" || operation == "END") {
                    continue;
                } else {
                    std::cerr << "Erro na linha " << line_counter << "! Operação não indentificada!\n";   
                }
            }
        }

        line_counter++;
    }

    // Criando e escrevendo no arquivo objeto
    std::string temp, obj_code_name = file_name.substr(0, file_name.size()-4) + ".obj";
    std::ofstream obj_code("./" + obj_code_name);

    if (has_link) {
        for (auto d : definitions_table) {
            temp = "D, " + d.first + " " + std::to_string(d.second);
            obj_code << temp + "\n";
        }

        for (auto u : usage_table) {
            for (int i = 0; i < u.second.size(); i++) {
                temp = "U, " + u.first + " " + std::to_string(u.second[i]);
                obj_code << temp + "\n";    
            }
        }

        obj_code << "R, " + bit_map + "\n";
    }

    if (obj_code.is_open()) {
        obj_code << machine_code;
    }

    obj_code.close();

    return usage_table;
}


int main(int argc, char* argv[]) {
    std::string file_name = argv[1];
   
    std::ifstream input_file(file_name);

    if (!input_file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << file_name << '\n';
        return 1;
    }
    
    if (file_name.substr(file_name.size()-4, 4) == ".asm") {
        pre_processing(file_name, input_file);
    } else if (file_name.substr(file_name.size()-4, 4) == ".pre") {
        std::map<std::string, std::pair<int, bool>> symbol_table;
        std::map<int, int*> symbol_values;
        std::map<std::string, int> definitions_table;
        std::map<std::string, std::vector<int>> usage_table;

        std::tie(symbol_table, symbol_values, definitions_table) = first_pass(input_file);
        std::ifstream input_file(file_name);
        usage_table = second_pass(file_name, input_file, symbol_table, symbol_values, definitions_table);
        
        input_file.close();
    } else {
        std::cerr << "Erro: arquivo(s) inválido(s)!\n";
        return 1;
    }

    return 0;
}