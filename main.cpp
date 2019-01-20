#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>

#define PREF_TEMP_FILE          "TEMP_FILE_"
#define MAX_LENGTH_FILE_NAME    100
#define MEMORY_SIZE             1024 // Mb
#define ll                      long long

int num_file = 0;

int cmp(char* &a,  char* &b) {
    return strcmp(a, b) < 0;
}

template<class T>
void resize(T*& temp_data, long long& size_temp) {
    if (temp_data == NULL) {
        temp_data = (T*) malloc(sizeof(T) * size_temp);
    } else {
        temp_data = (T*) realloc(temp_data, sizeof(T) * size_temp);
    }
    if (!temp_data) {
        std::cerr << "Memory is full!\n Ctrl + C to exit!\n";
        exit(0);
    }
}

void merge_files(int x, int y, char* fn) {
    char file_name_x[MAX_LENGTH_FILE_NAME];
    char file_name_y[MAX_LENGTH_FILE_NAME];
    sprintf(file_name_x, "%s%d.txt", PREF_TEMP_FILE, x);
    sprintf(file_name_y, "%s%d.txt", PREF_TEMP_FILE, y);

    std::ifstream ifs_x (file_name_x, std::ios::binary);
    std::ifstream ifs_y (file_name_y, std::ios::binary);
    std::ofstream ofs_fn (fn, std::ios::binary);
    std::string str_x, str_y;

    while(!ifs_x.eof() || !ifs_y.eof()) {
        if (str_x == "" && !ifs_x.eof()) {
            getline(ifs_x, str_x);
        }
        if (str_y == "" && !ifs_y.eof()) {
            getline(ifs_y, str_y);
        }
        if (str_x == "" && str_y == "") {
            break;
        }
        if (str_x == "" || (str_y != "" && str_y < str_x)) {
            ofs_fn << str_y << "\n";
            str_y = "";
        } else {
            ofs_fn << str_x << "\n";
            str_x = "";
        }
    }
    ifs_x.close();
    ifs_y.close();
    ofs_fn.close();
    remove(file_name_x);
    remove(file_name_y);
}

int divide_merge_files(int l, int r, char* fn = 0) {
    if (l == r) return l;
    int mid = (l + r) / 2;
    int x = divide_merge_files(l, mid);
    int y = divide_merge_files(mid + 1, r);
    int res = num_file++;
    char file_name[MAX_LENGTH_FILE_NAME];
    if (fn) {
        strcpy(file_name, fn);
    } else {
        sprintf(file_name, "%s%d.txt", PREF_TEMP_FILE, res);
    }
    merge_files(x, y, file_name);
    return res;
}

void check_file_size(ll file_size) {

    ll file_size_limit = 1;
    file_size_limit <<= 32;

    if (file_size < 0) std::cerr << "File have no data!";
    if (file_size > file_size_limit) std::cerr << "File should smaller than 200G!";

    if ((file_size >> 20) > 0)
        std::cout << "File size: "<< (file_size >> 20) << "Mb\n";
    else if ((file_size >> 10) > 0)
        std::cout << "File size: "<< (file_size >> 10) << "Kb\n";
    else
        std::cout << "File size: "<< file_size << "B\n";
}

void sort_lines_in_big_file(char* input_file, char* output_file, int mem_limit) {

    std::ifstream ifs (input_file, std::ifstream::binary);
    if (!ifs) {
        std::cerr << "File " << input_file << " not found!\n";
        return;
    }
    std::cout << "Get file size!\n";
    ifs.seekg (0, ifs.end);
    ll file_size = ifs.tellg();
    ifs.seekg (0, ifs.beg);

    check_file_size(file_size);

    mem_limit <<= 20;
    int standard_block_size = mem_limit / 2;
    ll nline_cap = 1;
    int nline = 0;
    std::string line;
    int current_block_size = 0;
    char** lines = (char**) malloc( sizeof(char*) * (nline_cap));

    std::cout << "Starting read file, dividing and sorting lines into block size of " << (standard_block_size >> 20) << " Mb!\n";
    while(!ifs.eof()) {
        getline(ifs, line);
        if (!line.empty()) {
            if (nline == nline_cap) resize(lines, nline_cap <<= 1);
            lines[nline] = (char*) malloc(sizeof(char) * (line.size() + 1));
            strcpy(lines[nline], line.data());
            nline++;
            current_block_size += (line.size() + 1);
        }
        if (current_block_size + sizeof(char*) * nline_cap >= standard_block_size || ifs.eof()) {
            char temp_file_name[MAX_LENGTH_FILE_NAME];
            sprintf(temp_file_name, "%s%d.txt", PREF_TEMP_FILE, num_file++);
            std::ofstream ofs (temp_file_name, std::ofstream::binary);
            if (!ofs) {
                std::cerr << "Can not open/create temp file with name " << input_file << "!\n";
                return;
            }
            std::sort(lines, lines + nline, cmp);

            for(int i = 0; i < nline; i++) {
                ofs << lines[i] << "\n";
                free(lines[i]);
            }
            free(lines);
            current_block_size = 0;
            nline = 0;
            nline_cap = 1;
            ofs.close();
            resize(lines = 0, nline_cap = 1);
        }
    }
    ifs.close();

    std::cout << "Starting merge files!\n";
    if (num_file == 1) { // only have one temp file divided, rename to out file
        char temp_file_name[MAX_LENGTH_FILE_NAME];
        sprintf(temp_file_name, "%s%d.txt", PREF_TEMP_FILE, num_file - 1);
        int result = rename( temp_file_name , output_file );

        if (result != 0) {
            std::cout << "Can not rename file on result of exist only one temp file," <<
                      " try option merge file!\n";
            sprintf(temp_file_name, "%s%d.txt", PREF_TEMP_FILE, num_file++);
            FILE* temp_file = fopen(temp_file_name, "wb");
            if (temp_file == NULL) {
                std::cerr << "Error in opening file " << temp_file_name << "\n";
                exit(0);
            }
            fclose(temp_file);
            divide_merge_files(0, num_file - 1, output_file);
        }
    } else if (num_file > 1) { // merge files
        divide_merge_files(0, num_file - 1, output_file);
    }
    num_file = 0;
    std::cout << "Done!\n";
    std::cout << "Output file: "<< output_file << "\n";
}


int main(int argc, char *argv[]) {
    char input_file[MAX_LENGTH_FILE_NAME];
    char output_file[MAX_LENGTH_FILE_NAME];
    if (argc > 1) {
        strcpy(input_file, argv[1]);
    } else {
        strcpy(input_file, "input.txt");
    }

    if (argc > 2) {
        strcpy(output_file, argv[2]);
    } else {
        strcpy(output_file, "output.txt");
    }
    std::cout << "Input file: "<< input_file << "\n";

    sort_lines_in_big_file(input_file, output_file, MEMORY_SIZE);

    std::cerr << "\nTime elapsed: " << 1000 * clock() / CLOCKS_PER_SEC << "ms\n";
    return 0;
}
