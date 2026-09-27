// simple ideation and understanding of starting to understand pratt parsing proof
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <utility>
#include <vector>

using std::vector;

enum TokenType{
    TokenType_none,
    TokenType_number,
    TokenType_add,
    TokenType_mul,
    TokenType_equal,
    TokenType_eof,
    TokenType_count,
};

struct Token {
    TokenType type;
    char * at;
    int len;
    void print(){
        printf("type %d: at : %.*s\n", type, len, at);
    }
};

vector<Token> tokenize(char * text){
    vector<Token> tokens;
    tokens.resize(0);
    char * at = text;
    while(*at){
        while (*at == ' ') at++;
        char ch = *at;
        Token t = {};
        switch(ch) {
            case '=': t.type = TokenType_equal; t.at = at; t.len = 1; break;
            case '+': t.type = TokenType_add; t.at = at; t.len = 1; break;
            case '*': t.type = TokenType_mul; t.at = at; t.len = 1; break;
            default: t.type = TokenType_number; t.at = at; t.len = 1; break;
        } 
        at++;
        tokens.push_back(t);
    }
    return tokens;
}

int op_val(TokenType type){
    switch(type) {
        case TokenType_add: return 2;
        case TokenType_mul: return 3;
        case TokenType_equal: return 1;
        default: return 0;
    }
}

const char * op_str(TokenType type){
    switch(type) {
        case TokenType_add: return "+";
        case TokenType_mul: return "*";
        case TokenType_equal: return "=";
        default: return "not_operator";
    }
}



struct Leaf { int value; };
struct Ex;
struct Binary{ Ex *left, *right; TokenType op; };

enum ExpressionType{
    ExpressionType_none,
    ExpressionType_leaf,
    ExpressionType_binary,
    ExpressionType_count
};

struct Ex {
    ExpressionType type;
    union { Leaf leaf; Binary binary; };
    void print() {
        if (type == ExpressionType_leaf){
            printf(" %d", leaf.value);
        }
        if (type == ExpressionType_binary){
            printf("(%s ", op_str(binary.op));
            binary.left->print();
            binary.right->print();
            printf(")");
        }
        else {}
    }
};

struct Parser {
    int idx;
    vector<Token> tokens;
    Token curr() const {
        return tokens[idx];
    }
    Token peek(int x) {
        if (x + idx < tokens.size()) return tokens[idx + x];
        return tokens[tokens.size() - 1];
    }
    void next(int x = 1) {
        if (idx + x  < tokens.size()) idx += x;
        else idx = tokens.size() - 1;
    }
};


Ex parse_leaf(const Token & token) {
    Ex ex = {};
    char buf[1024];
    strncpy(buf, token.at, token.len);
    ex.leaf.value = atoi(buf);
    ex.type = ExpressionType_leaf;
    return ex;
}


Ex create_binary_expression(Ex * left, Ex * right, TokenType op){
    Ex ex  ={};
    ex.type = ExpressionType_binary;
    ex.binary.left = (Ex *) malloc(sizeof(Ex));
    ex.binary.right = (Ex *) malloc(sizeof(Ex));
    *ex.binary.left = *left;
    *ex.binary.right = *right;
    ex.binary.op = op;
    return ex;
} ;



Ex parse_expression(Parser & parser, int min_prec) {
    
    Ex left = parse_leaf(parser.curr());
    parser.next();

    printf("min_val : %d left: ", min_prec);
    left.print();
    printf("\n");

    while(parser.curr().type != TokenType_eof){
        TokenType op = parser.curr().type;
        if (op_val(op) == 0 || op_val(op) < min_prec) break;
        parser.next();

        Ex right = parse_expression(parser, op_val(op));
        printf("right: ");
        right.print();
        printf("\n");
        left = create_binary_expression(&left, &right, op);
        printf("new left: ");
        left.print();
        printf("\n");
    }

    return left;
}

int main(){
    auto tokens = tokenize((char *)"1 + 5 * 4 + 4 * 4");
    for(auto token : tokens){
        token.print();
    }
    Parser parser;
    parser.idx = 0;
    parser.tokens = std::move(tokens);
    Ex res = parse_expression(parser, 0);
    return 0;
}

