#include "Node.hpp"
#include "huffman.h"
#include <cstring>
#include <unordered_map>

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: ./decompress <file name> <output file name>\n");
        return -1;
    }

    char* inputFileName = (char*)malloc(sizeof(char)*strlen(argv[1]));
    strcpy(inputFileName, argv[1]);

    char* outputFileName = (char*)malloc(sizeof(char)*strlen(argv[2]));
    strcpy(outputFileName, argv[2]);

    printf("Opening input file '%s'\n", inputFileName);
    FILE* inputFile = fopen(inputFileName, "rb");
    if (inputFile == NULL) {
        printf("Failed to open input file '%s'\n", inputFileName);
        return -1;
    }

    printf("Opening output file '%s'\n", outputFileName);
    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        printf("Failed to open output file '%s'\n", outputFileName);
        return -1;
    }

    int padding = 0;
    fread(&padding, sizeof(char), 1, inputFile);
    printf("Padding: %d\n", padding);

    int numUnique = 0;
    fread(&numUnique, sizeof(char), 1, inputFile);
    printf("Unique characters: %d\n", numUnique);
    if (numUnique == 0) numUnique = 256;

    std::unordered_map<std::string, char> codes;
    for (unsigned int i = 0; i < numUnique; i++) {
        charCode cC;
        fread(&cC, sizeof(charCode), 1, inputFile);
        std::string code(cC.code);
        codes[code] = cC.ch;
    }

    // printf("Map:\n");
    // for (auto pair : codes) {
    //     std::cout << "[" << pair.first << "," << pair.second << "]" << std::endl;
    // }

    int numIterations = 0;
    char byte = 0;
    while(fread(&byte, sizeof(char), 1, inputFile) != 0u) numIterations++;
    fseek(inputFile, -sizeof(char)*numIterations, SEEK_CUR);
    printf("Num iterations: %d\n", numIterations);

    std::string code;
    int currentBit = 0;
    int currentIter = 0;
    byte = 0;
    while (fread(&byte, sizeof(char), 1, inputFile)) {
        currentIter++;
        for (unsigned int bitCount = 0; bitCount < byteLen; bitCount++) {
            // last iteration -> discard padding
            if (currentIter == numIterations && bitCount == byteLen - padding) { 
                break;
            }
            currentBit = (byte >> (byteLen - 1 - bitCount) & 1);
            code.append(std::to_string(currentBit));
            if (codes.find(code) != codes.end()) {
                fwrite(&codes[code], sizeof(char), 1, outputFile);
                code.clear();
            }
        }
    }

    fclose(outputFile);
    fclose(inputFile);
    return 0;
}
