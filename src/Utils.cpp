#include<fstream>
#include"Utils.h"

void SaveJsonToFile(const std::string& filename, const char* jsonData) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << jsonData;
        outFile.close();
        std::cout << "JSON data save to file " << filename << std::endl;
    } else {
        std::cerr << "cannot open " << filename << std::endl;
    }
}