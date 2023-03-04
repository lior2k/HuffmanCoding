#include "Node.hpp"
#include <fstream>
#include <unordered_map>
using namespace std;

#define codeLen 16
#define byteLen 8

struct charCode {
    char ch;
    char code[codeLen]; // why did i decide on 16?
};

int main(int argc, char const *argv[]) {
    FILE* inputFile = fopen("test_file_encoded.txt", "rb");
    int numUnique = 0;
    fread(&numUnique, sizeof(char), 1, inputFile);

    printf("Unique characters: %d\n", numUnique);

    unordered_map<string, char> codes;
    for (unsigned int i = 0; i < numUnique; i++) {
        charCode cC;
        fread(&cC, sizeof(charCode), 1, inputFile);
        string code(cC.code);
        codes[code] = cC.ch;
    }

    printf("Map:\n");
    for (auto pair : codes) {
        cout << "[" << pair.first << "," << pair.second << "]" << endl;
    }

    FILE *outputFile = fopen("test_file_decoded.txt", "w");

    string code = "";
    int currentBit;
    char byte = 0;
    while (fread(&byte, 1, 1, inputFile)) {
        printf("%d\n", byte);
        for (unsigned int bitCount = 0; bitCount < byteLen; bitCount++) {
            currentBit = (byte >> (7 - bitCount) & 1);
            printf("%d ", currentBit);
            code.append(to_string(currentBit));
            if (codes.find(code) != codes.end()) {
                fwrite(&codes[code], 1, 1, outputFile);
                printf("%c", codes[code]);
                code.clear();
            }
        }
        printf("\n");
    }
    fclose(outputFile);
    fclose(inputFile);
    return 0;
}
