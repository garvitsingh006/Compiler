
#include <bits/stdc++.h>
using namespace std;

struct Token {
    string type, value;
};

struct Node {
    string value;
    vector<Node*> children;
    Node(string v) : value(v) {}
};

vector<Token> tokens;
int pos = 0;
map<string, vector<vector<string>>> grammar;
map<string, set<string>> FIRST, FOLLOW;
map<pair<string, string>, vector<string>> table;

// PHASE 1: LEXICAL ANALYZER

void lexicalAnalyzer(string s) {
    for (int i = 0; i < s.size();) {
        if (isspace((unsigned char)s[i])) i++;
        else if (isalpha((unsigned char)s[i]) || s[i] == '_') {
            string word;
            while (i < s.size() && (isalnum((unsigned char)s[i]) || s[i] == '_'))
                word += s[i++];
            tokens.push_back({"IDENTIFIER", word});
        }
        else if (isdigit((unsigned char)s[i])) {
            string num;
            while (i < s.size() && isdigit((unsigned char)s[i]))
                num += s[i++];
            tokens.push_back({"NUMBER", num});
        }
        else if (string("+-*/=();").find(s[i]) != string::npos)
            tokens.push_back({"OPERATOR", string(1, s[i++])});
        else throw runtime_error("Invalid character: " + string(1, s[i]));
    }
    tokens.push_back({"END", "$"});
}

// PHASE 2: DEFINE GRAMMAR

void initializeGrammar() {
    grammar["S"] = {{"C", "A"}};
    grammar["A"] = {{";", "B"}, {"ε"}};
    grammar["B"] = {{"C", "A"}, {"ε"}};
    grammar["C"] = {{"id", "D"}, {"num", "H", "F"}, {"(", "E", ")", "H", "F"}};
    grammar["D"] = {{"=", "E"}, {"H", "F"}};
    grammar["E"] = {{"G", "F"}};
    grammar["F"] = {{"+", "G", "F"}, {"-", "G", "F"}, {"ε"}};
    grammar["G"] = {{"I", "H"}};
    grammar["H"] = {{"*", "I", "H"}, {"/", "I", "H"}, {"ε"}};
    grammar["I"] = {{"id"}, {"num"}, {"(", "E", ")"}};
}

bool isNonTerminal(string s) {
    return grammar.count(s);
}

// PHASE 3: FIRST SET

set<string> firstOfSequence(vector<string> seq) {
    set<string> result;
    bool nullable = true;

    for (string symbol : seq) {
        if (!isNonTerminal(symbol)) {
            result.insert(symbol);
            nullable = false;
            break;
        }

        for (string x : FIRST[symbol])
            if (x != "ε") result.insert(x);

        if (!FIRST[symbol].count("ε")) {
            nullable = false;
            break;
        }
    }

    if (nullable) result.insert("ε");
    return result;
}

void calculateFIRST() {
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto &rule : grammar) {
            for (auto production : rule.second) {
                for (string x : firstOfSequence(production))
                    if (FIRST[rule.first].insert(x).second)
                        changed = true;
            }
        }
    }
}

// PHASE 4: FOLLOW SET

void calculateFOLLOW() {
    FOLLOW["S"].insert("$");
    bool changed = true;

    while (changed) {
        changed = false;
        for (auto &rule : grammar) {
            string lhs = rule.first;

            for (auto production : rule.second) {
                for (int i = 0; i < production.size(); i++) {
                    string symbol = production[i];
                    if (!isNonTerminal(symbol)) continue;

                    vector<string> remaining(
                        production.begin() + i + 1, production.end()
                    );

                    set<string> temp = firstOfSequence(remaining);

                    for (string x : temp)
                        if (x != "ε" && FOLLOW[symbol].insert(x).second)
                            changed = true;

                    if (temp.count("ε"))
                        for (string x : FOLLOW[lhs])
                            if (FOLLOW[symbol].insert(x).second)
                                changed = true;
                }
            }
        }
    }
}

// DISPLAY FIRST AND FOLLOW

void displaySets() {
    cout << "\nFIRST SETS:\n";
    for (auto &x : grammar) {
        cout << "FIRST(" << x.first << ") = { ";
        for (string s : FIRST[x.first]) cout << s << " ";
        cout << "}\n";
    }

    cout << "\nFOLLOW SETS:\n";
    for (auto &x : grammar) {
        cout << "FOLLOW(" << x.first << ") = { ";
        for (string s : FOLLOW[x.first]) cout << s << " ";
        cout << "}\n";
    }
}

// PHASE 5: GENERATE LL(1) PARSING TABLE

void generateParsingTable() {
    for (auto &rule : grammar) {
        string lhs = rule.first;

        for (auto production : rule.second) {
            set<string> temp = firstOfSequence(production);

            for (string t : temp) {
                if (t == "ε") continue;
                auto key = make_pair(lhs, t);
                if (table.count(key))
                    throw runtime_error("Grammar is not LL(1)");
                table[key] = production;
            }

            if (temp.count("ε")) {
                for (string t : FOLLOW[lhs]) {
                    auto key = make_pair(lhs, t);
                    if (table.count(key))
                        throw runtime_error("Grammar is not LL(1)");
                    table[key] = production;
                }
            }
        }
    }
}

// DISPLAY PARSING TABLE

void displayTable() {
    vector<string> terminals = {
        "id", "num", "+", "-", "*", "/", "(", ")", "=", ";", "$"
    };

    cout << "\nLL(1) PARSING TABLE:\n";
    int chunkSize = 4;

    for (int start = 0; start < terminals.size(); start += chunkSize) {
        int end = min(start + chunkSize, (int)terminals.size());

        cout << "\n" << left << setw(20) << "NON-TERMINAL";
        for (int i = start; i < end; i++)
            cout << setw(22) << terminals[i];

        cout << "\n" << string(20 + (end - start) * 22, '-') << "\n";

        for (auto &rule : grammar) {
            cout << left << setw(20) << rule.first;

            for (int i = start; i < end; i++) {
                auto key = make_pair(rule.first, terminals[i]);
                string production = "-";

                if (table.count(key)) {
                    production = "";
                    for (string s : table[key])
                        production += s + " ";
                }
                cout << setw(22) << production;
            }
            cout << "\n";
        }
        cout << "\n";
    }
}

// PHASE 6: TOKEN LOOKAHEAD

string getLookahead() {
    if (tokens[pos].type == "IDENTIFIER") return "id";
    if (tokens[pos].type == "NUMBER") return "num";
    return tokens[pos].value;
}

// PHASE 7: LL(1) PREDICTIVE PARSER

Node* parse() {
    Node* root = new Node("Program");
    Node* start = new Node("S");
    root->children.push_back(start);

    stack<pair<string, Node*>> st;
    st.push({"$", nullptr});
    st.push({"S", start});
    pos = 0;

    while (!st.empty()) {
        string top = st.top().first;
        Node* currentNode = st.top().second;
        st.pop();

        string lookahead = getLookahead();

        if (top == "$") {
            if (lookahead == "$") break;
            throw runtime_error("Unexpected token: " + tokens[pos].value);
        }

        if (!isNonTerminal(top)) {
            if (top == lookahead) {
                if (top == "id" || top == "num")
                    currentNode->value = tokens[pos].value;
                pos++;
            }
            else {
                throw runtime_error("Expected '" + top +
                    "' but found '" + tokens[pos].value + "'");
            }
        }
        else {
            auto key = make_pair(top, lookahead);

            if (!table.count(key))
                throw runtime_error("No rule in parsing table for [" +
                    top + ", " + lookahead + "]");

            vector<string> production = table[key];

            if (production.size() == 1 && production[0] == "ε") {
                currentNode->children.push_back(new Node("ε"));
            }
            else {
                vector<Node*> children;

                for (string symbol : production) {
                    Node* child = new Node(symbol);
                    currentNode->children.push_back(child);
                    children.push_back(child);
                }

                for (int i = production.size() - 1; i >= 0; i--)
                    st.push({production[i], children[i]});
            }
        }
    }

    if (tokens[pos].type != "END")
        throw runtime_error("Unexpected extra input");

    return root;
}

// PHASE 8: DISPLAY PARSE TREE

void printTree(Node* root, string prefix = "",
               bool isLast = true, bool isRoot = true) {
    if (!root) return;

    if (isRoot) cout << root->value << endl;
    else cout << prefix << (isLast ? "`-- " : "|-- ")
              << root->value << endl;

    for (int i = 0; i < root->children.size(); i++) {
        string newPrefix = prefix;
        if (!isRoot) newPrefix += (isLast ? "    " : "|   ");

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

        initializeGrammar();

        cout << "\n--- PHASE 2: FIRST AND FOLLOW ---\n";
        calculateFIRST();
        calculateFOLLOW();
        displaySets();

        cout << "\n--- PHASE 3: LL(1) PARSING TABLE ---\n";
        generateParsingTable();
        displayTable();

        cout << "\n--- PHASE 4: SYNTAX ANALYSIS ---\n";
        Node* root = parse();
        cout << "\nSyntax Analysis Successful!\n";

        cout << "\n--- PARSE TREE ---\n";
        printTree(root);
    }
    catch (exception &e) {
        cout << "\nCompilation Error: " << e.what() << endl;
    }

    return 0;
}
