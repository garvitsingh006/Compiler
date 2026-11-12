
#include <bits/stdc++.h>
using namespace std;

struct Token {
    string type, value;
};

struct Node {
    string value;
    vector<Node*> children;

    Node(string v) {
        value = v;
    }
};

vector<Token> tokens;
int pos = 0;

// PHASE 1: LEXICAL ANALYZER

void lexicalAnalyzer(string s) {
    for (int i = 0; i < s.size();) {

        if (isspace(s[i])) {
            i++;
        }
        else if (isalpha(s[i]) || s[i] == '_') {
            string word;
            while (i < s.size() &&
                  (isalnum(s[i]) || s[i] == '_'))
                word += s[i++];

            tokens.push_back({"IDENTIFIER", word});
        }
        else if (isdigit(s[i])) {
            string num;
            while (i < s.size() && isdigit(s[i]))
                num += s[i++];

            tokens.push_back({"NUMBER", num});
        }
        else if (string("+-*/=();").find(s[i]) != string::npos) {
            tokens.push_back({"OPERATOR", string(1, s[i])});
            i++;
        }
        else {
            throw runtime_error("Invalid character: " + string(1, s[i]));
        }
    }

    tokens.push_back({"END", "$"});
}

// PHASE 2: SYNTAX ANALYZER

string current() {
    return tokens[pos].value;
}

Node* expression();
Node* statement();

Node* factor() {
    Node* node = new Node("Factor");

    if (tokens[pos].type == "IDENTIFIER" ||
        tokens[pos].type == "NUMBER") {

        node->children.push_back(new Node(current()));
        pos++;
    }
    else if (current() == "(") {
        node->children.push_back(new Node("("));
        pos++;

        node->children.push_back(expression());

        if (current() != ")")
            throw runtime_error("Missing closing bracket");

        node->children.push_back(new Node(")"));
        pos++;
    }
    else {
        throw runtime_error("Invalid factor: " + current());
    }

    return node;
}

Node* term() {
    Node* node = new Node("Term");
    node->children.push_back(factor());

    while (current() == "*" || current() == "/") {
        node->children.push_back(new Node(current()));
        pos++;
        node->children.push_back(factor());
    }

    return node;
}

Node* expression() {
    Node* node = new Node("Expression");
    node->children.push_back(term());

    while (current() == "+" || current() == "-") {
        node->children.push_back(new Node(current()));
        pos++;
        node->children.push_back(term());
    }

    return node;
}

Node* statement() {
    Node* node = new Node("Statement");

    if (tokens[pos].type == "IDENTIFIER" &&
        tokens[pos + 1].value == "=") {

        node->children.push_back(new Node(current()));
        pos++;

        node->children.push_back(new Node("="));
        pos++;

        node->children.push_back(expression());
    }
    else {
        node->children.push_back(expression());
    }

    return node;
}

Node* program() {
    Node* root = new Node("Program");

    root->children.push_back(statement());

    while (current() == ";") {
        pos++;

        if (current() == "$")
            break;

        root->children.push_back(new Node(";"));
        root->children.push_back(statement());
    }

    if (current() != "$")
        throw runtime_error("Unexpected token: " + current());

    return root;
}

// DISPLAY PARSE TREE



void printTree(Node* root, string prefix = "", bool isLast = true, bool isRoot = true) {
    if (!root) return;

    if (isRoot)
        cout << root->value << endl;
    else
        cout << prefix << (isLast ? "`-- " : "|-- ")
             << root->value << endl;

    for (int i = 0; i < root->children.size(); i++) {
        string newPrefix = prefix;

        if (!isRoot)
            newPrefix += (isLast ? "    " : "|   ");

        printTree(root->children[i], newPrefix,
                  i == root->children.size() - 1, false);
    }
}



// MAIN FUNCTION

int main() {
    string input;

    cout << "Enter source code: ";
    getline(cin, input);

    try {
        cout << "\n--- PHASE 1: LEXICAL ANALYSIS ---\n";

        lexicalAnalyzer(input);

        cout << "\nTOKEN STREAM:\n";

        for (int i = 0; i < tokens.size() - 1; i++)
            cout << "<" << tokens[i].type << ", "
                 << tokens[i].value << ">\n";

        cout << "\n--- PHASE 2: SYNTAX ANALYSIS ---\n";

        pos = 0;
        Node* root = program();

        cout << "Syntax Analysis Successful!\n";

        cout << "\nPARSE TREE:\n";
        printTree(root);
    }
    catch (exception &e) {
        cout << "\nCompilation Error: " << e.what() << endl;
    }

    return 0;
}
