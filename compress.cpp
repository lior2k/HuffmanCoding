#include "Node.hpp"
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstring>
using namespace std;

#define codeLen 16
#define byteLen 8

struct charCode {
    char ch;
    char code[codeLen]; // why did i decide on 16?
};

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

int main(int argc, char const *argv[]) {
    /*
    Begin by reading the file and constructing a hashmap containing the letters
    and their frequencies - eg - [ (A:3), (B:8), ... , (Z:2) ]
    */
    unordered_map<char, int> hm;
    string line;
    ifstream fin;
    fin.open("test_file.txt");
    while (getline(fin, line)) {
        for (char ch : line) {
            if (hm.find(ch) == hm.end()) {
                hm[ch] = 1;
            } else {
                hm[ch]++;
            }
        }
    }

    // Construct the leaf nodes of the Huffman binary tree.
    vector<Node*> nodes;
    for (auto pair : hm) {
        nodes.emplace_back(new Node(pair.first, pair.second));
    }

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
    unordered_map<char, string> codes = treeToMap(root);

    FILE *outputFile = fopen("test_file_encoded.txt", "w");

    // write number of unique characters.
    int numUnique = codes.size();
    fwrite(&numUnique, sizeof(char), 1, outputFile);

    // write the characters and their codes.
    charCode cC;
    for (auto pair : codes) {
        cC.ch = pair.first;
        strcpy(cC.code, pair.second.data());
        fwrite(&cC, sizeof(cC), 1, outputFile);
        if (strlen(pair.second.data()) > codeLen) {
            cout << "warning: code " << pair.second << "length > 16" << endl;
        }
    }

    fin.clear();
    fin.seekg(0, ios::beg);
    int currentBit = 0;
    int bitBuffer = 0;
    string code;
    while (getline(fin, line)) {
        for (char ch : line) {
            code = codes[ch];
            for (char bit : code) {
                int ibit = bit - '0';
                if (ibit) {
                    bitBuffer = bitBuffer + (1 << (byteLen - 1 - currentBit));
                }
                currentBit++;
                if (currentBit == byteLen) {
                    fwrite(&bitBuffer, 1, 1, outputFile);
                    cout << bitBuffer << endl;
                    currentBit = 0;
                    bitBuffer = 0;
                }
            }
        }
    }
    while (1) {
        currentBit++;
        if (currentBit == byteLen) {
            fwrite(&bitBuffer, 1, 1, outputFile);
            cout << bitBuffer << endl;
            break;
        }
    }

    fin.close();
    fclose(outputFile);
    return 0;
}
