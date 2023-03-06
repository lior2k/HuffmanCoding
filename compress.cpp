#include "Node.hpp"
#include "huffman.h"
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstring>
using namespace std;

void treeToMap(unordered_map<char, string> &codes, string currentCode, Node* node) {
    if (node == nullptr) {
        return;
    }
    if (node->getChar() != '#') {
        codes[node->getChar()] = currentCode;
    }
    treeToMap(codes, currentCode + "0", node->getLeft());
    treeToMap(codes, currentCode + "1", node->getRight());
}

/*
Traverse the huffman binary tree and extract chars with their respective codes into
an unordered map.
Wrapper functions takes the root node of the tree and calls the recursive method with
a starting code value 0 for left turn and 1 for right turn. The recursive method then
proceeds to add 0s for left and 1s for right until reaching a leaf with a character,
then adding that character with the assembeled code into the map. 
*/
unordered_map<char, string> treeToMap(Node* root) {
    unordered_map<char, string> codes;
    treeToMap(codes, "0", root->getLeft());
    treeToMap(codes, "1", root->getRight());
    return codes;
}


void printBT(const string& prefix, const Node* node, bool isLeft) {
    if (node != nullptr) {
        cout << prefix;
        cout << (isLeft ? "├──" : "└──" );
        // print the value of the node
        cout << node->getChar() << ',' << node->getFrequency() << endl;
        // enter the next tree level - left and right branch
        printBT( prefix + (isLeft ? "│   " : "    "), node->getLeft(), true);
        printBT( prefix + (isLeft ? "│   " : "    "), node->getRight(), false);
    }
}

// Binary tree print function taken from Stackoverflow to debug the huffman generated tree.
void printBT(const Node* node) {
    printBT("", node, false);    
}

/*
Functor to compare node (character) frequencies, used as input for the sort method,
for some reason the implementation of '<' operator didn't suffice.
*/
struct less_then {
    inline bool operator() (const Node* n1, const Node* n2) {
        return n1->getFrequency() < n2->getFrequency();
    } 
};

/*
Multiply each character's code with it's frequency to determine number of required bits
and calculate the required padding to fill a whole byte.
*/
int calcPadding(unordered_map<char, string>& codes ,unordered_map<char, int>& frequencyMap) {
    unsigned long sum = 0;
    for (auto pair : codes) {
        sum += pair.second.size() * frequencyMap[pair.first];
    }
    return byteLen - (sum % 8);
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: ./compress <file name> <output file name>\n");
        return -1;
    }

    char* inputFileName = (char*)malloc(sizeof(char)*strlen(argv[1]));
    strcpy(inputFileName, argv[1]);

    char* outputFileName = (char*)malloc(sizeof(char)*strlen(argv[2]));
    strcpy(outputFileName, argv[2]);

    printf("Opening input file '%s'\n", inputFileName);
    FILE *inputFile = fopen(inputFileName, "r");
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

    /*
    Begin by reading the file and constructing a hashmap containing the letters
    and their frequencies - eg - [ (A:3), (B:8), ... , (Z:2) ]
    */
    printf("Reading input file - creating huffman nodes\n");
    char ch = 0;
    unordered_map<char, int> frequencyMap;
    while (fread(&ch, sizeof(char), 1, inputFile) != 0u) {
        if (frequencyMap.find(ch) == frequencyMap.end()) {
            frequencyMap[ch] = 1;
        } else {
            frequencyMap[ch]++;
        }
    }

    // Construct the leaf nodes of the Huffman binary tree.
    vector<Node*> nodes;
    for (auto pair : frequencyMap) {
        nodes.emplace_back(new Node(pair.first, pair.second));
    }

    printf("Generating frequency tree\n");
    // Sort the nodes.
    sort(nodes.begin(), nodes.end(), less_then());
    // Construct the binary tree in buttom up manner by combining two of
    // the lowest frequency nodes each iteration. 
    while(nodes.size() > 1) {
        Node* n1 = nodes.front();
        nodes.erase(nodes.begin());
        Node* n2 = nodes.front();
        nodes.erase(nodes.begin());
        Node* newNode = new Node('#', n1->getFrequency() + n2->getFrequency(), n1, n2);
        bool inserted = false;
        for (unsigned int i = 0; i < nodes.size(); i++) {
            if (*newNode < *nodes[i]) {
                nodes.insert(nodes.begin() + i, newNode);
                inserted = true;
                break;
            }
        }
        // newNode frequency (the joined frequency of the subnodes) is bigger
        // then all other frequencies so just push back.
        if (!inserted) {
            nodes.push_back(newNode);
        }
        
    }

    // Root of the Huffman tree = the only node remaining in the list of nodes.
    Node *root = nodes[0];

    // Traverse the Huffman tree and generate a map of letters with their respective codes.
    printf("Creating char->code map\n");
    unordered_map<char, string> codes = treeToMap(root);


    // write number of padding bits that will fill the last byte.
    printf("Calculating padding\n");
    int padding = calcPadding(codes, frequencyMap);
    printf("Writing file header:\n");
    fwrite(&padding, sizeof(char), 1, outputFile);

    // write number of unique characters.
    int numUnique = codes.size() == 256 ? 0 : codes.size(); // if 256 alias as 0 (because 256 is 9 bits)
    fwrite(&numUnique, sizeof(char), 1, outputFile);

    // write the characters and their codes.
    charCode cC{};
    for (auto pair : codes) {
        cC.ch = pair.first;
        strcpy(cC.code, pair.second.data());
        fwrite(&cC, sizeof(cC), 1, outputFile);
        if (strlen(pair.second.data()) > codeLen) {
            cout << "warning: code " << pair.second << "length > 16" << endl;
        }
    }
    printf("Padding: %d\nCharacters: %d\n%d charCode structs\n", padding, numUnique, numUnique);

    // write encoded content
    printf("Writing encoded data\n");
    fseek(inputFile, 0, SEEK_SET);
    int currentBit = 0;
    unsigned char bitBuffer = 0;
    string code;
    while (fread(&ch, sizeof(char), 1, inputFile) ) {
        code = codes[ch];
        for (char bit : code) {
            int ibit = bit - '0';
            if (ibit) {
                bitBuffer = bitBuffer + (1 << (byteLen - 1 - currentBit));
            }
            currentBit++;
            if (currentBit == byteLen) {
                fwrite(&bitBuffer, sizeof(char), 1, outputFile);
                currentBit = 0;
                bitBuffer = 0;
            }
        }
    }
    // write padding
    while (1) {
        currentBit++;
        if (currentBit == byteLen) {
            fwrite(&bitBuffer, sizeof(char), 1, outputFile);
            break;
        }
    }

    fclose(inputFile);
    fclose(outputFile);
    return 0;
}
