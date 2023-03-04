#include <iostream>

class Node {

    private:
        const char ch;
        int frequency;
        Node *left;
        Node *right;
    
    public:
        Node(char ch, int frequency = 0, Node *left = nullptr, Node *right = nullptr);
        char getChar() const;
        int getFrequency() const;
        void setLeft(Node *left);
        Node *getLeft() const;
        void setRight(Node *right);
        Node *getRight() const;

        bool operator < (const Node &other) const;
};

std::ostream & operator << (std::ostream &os, Node *node);