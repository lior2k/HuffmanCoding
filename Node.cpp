#include "Node.hpp"

Node::Node(const char ch, int frequency, Node *left, Node *right) : ch(ch), frequency(frequency), left(left), right(right) {};

char Node::getChar() const {
    return this->ch;
}

void Node::setLeft(Node *left) {
    this->left = left;
}

Node * Node::getLeft() const {
    return this->left;
}

void Node::setRight(Node *right) {
    this->right = right;
}

Node * Node::getRight() const {
    return this->right;
}

int Node::getFrequency() const {
    return this->frequency;
}

bool Node::operator < (const Node &other) const {
    return this->getFrequency() < other.getFrequency();
}

std::ostream & Node::operator << (std::ostream &os) {
    os << "[" << this->getChar() << "," << this->getFrequency() << "]";
    return os;
}