#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// util for parser
struct StringView {
    char * data;
    size_t len;
};

struct String {
    char * data;
    size_t len;
};
String load_entire_file(const char * file_path);

struct Cursor {
    char * at;
};
void eat_white_space(Cursor * cursor);

enum TokenType {
    TokenType_none,

    TokenType_comma,

    TokenType_opening_paran,
    TokenType_closing_paran,

    TokenType_opening_brace,
    TokenType_closing_brace,

    TokenType_plus,
    TokenType_minus,
    TokenType_astricks,
    TokneType_forward_slash, // can this be simply a 'divide', are there more uses for / other than divide and comment ? 

    TokenType_equal,
    TokenType_double_equal,

    TokenType_bang,
    TokenType_bang_equal,

    TokenType_double_quote,
    TokenType_semi_colon,

    TokenType_bool_true,
    TokenType_bool_false,

    TokenType_if,
    TokenType_else,
    TokenType_for,
    TokenType_return,
    TokenType_func,
    TokenType_print,
    TokenType_and,
    TokenType_or,
    
    TokenType_identifier,
    TokenType_string_literal,
    TokenType_number_literal,

    TokenType_eof,
};
bool is_binary_operator(TokenType type);
bool is_unary_operator(TokenType type);

struct Token {
    TokenType  type;
    StringView at;
};
bool is_alpha(char a);
bool is_number(char a);
bool is_white_space(char a);
void print_token(Token token);

struct ParsedTokens {
    Token * tokens;
    size_t count;
};

ParsedTokens fetch_tokens(String file_source);

struct Parser {
    ParsedTokens pt;
    size_t idx;

    Token curr() { return pt.tokens[idx]; }
    Token peek(int n) { return pt.tokens[idx + n]; }
    void next(int val = 1) { idx += val; }

    bool end() { return idx == pt.count; }
};

enum OperatorType {
    OperatorType_none,
    OperatorType_add,
    OperatorType_subtract,
    OperatorType_mult,
    OperatorType_divide,
    OperatorType_and,
    OperatorType_or,
    OperatorType_assign,
    OperatorType_count,
};
OperatorType get_operator_type(TokenType type);

struct Operator {
    OperatorType type;
};

struct NameExp {
    StringView value;
};

struct NumberLiteralExp {
    int value;
};

struct StringLiteralExp {
    StringView value;
};

struct Expression;

struct UniaryExp {
    Expression * exp;
    Operator opt;
};

struct BinaryExp {
    Expression * left;
    Expression * right;
    Operator opt;
};

struct CallExp {
    StringView call_name;
    Expression * args;
    int arg_count;
};

struct GroupExp {
    Expression * expressions;
    int expression_count;
};

enum ExpressionType {
    ExpressionType_none,

    ExpressionType_literal_number,
    ExpressionType_literal_string,

    ExpressionType_name,

    ExpressionType_binary,
    ExpressionType_unary,

    ExpressionType_call,

    ExpressionType_group,

    ExpressionType_count,
};

struct Expression {
    ExpressionType type;
    union {
        BinaryExp binary;
        UniaryExp unary;
        NumberLiteralExp number;
        StringLiteralExp string;
        NameExp name;
        CallExp call;
    } exp ;
};

void lpad(int pad) {
    for(int i = 0; i < pad; i++)
    printf(" ");
}

void print_expression(Expression exp, int depth = 0) {
    lpad(depth); printf("expressio type:%d\n", exp.type);
    if (exp.type == ExpressionType_name) {
        lpad(depth+1);printf("name: %.*s\n", (int)exp.exp.name.value.len, exp.exp.name.value.data);
    }
    else if (exp.type == ExpressionType_literal_number) {
        lpad(depth+1);printf("value: %d\n", exp.exp.number.value);
    }
    else if (exp.type == ExpressionType_unary) {
        lpad(depth+1);printf("operator: %d\n", exp.exp.unary.opt.type);
        lpad(depth+1);printf("right\n");
        print_expression(*exp.exp.unary.exp, depth + 1);
    }
    else if (exp.type == ExpressionType_literal_string) {
        lpad(depth+1);printf("value; %.*s\n", (int)exp.exp.string.value.len, exp.exp.string.value.data);
    }
    else if (exp.type == ExpressionType_binary) {
        lpad(depth+1);printf("operator: %d\n", exp.exp.binary.opt.type);
        lpad(depth+1);printf("left\n");
        print_expression(*exp.exp.binary.left, depth + 1);
        lpad(depth+1);printf("right\n");
        print_expression(*exp.exp.binary.right, depth + 1);
    }
}

enum StatementType {
    StatementType_none,
    StatementType_assign,
    StatementType_count,
};

struct AssignStat {
    StringView target; 
    Expression value;
};

struct IfStat {

};

struct Statement {
    StatementType type;
    union {
        AssignStat assign;
    } state ;
};

void print_statement(Statement stmt) {
    printf("stmt type: %d\n", stmt.type);
    if (stmt.type  == StatementType_assign) {
        printf("asign target: %.*s\n", (int)stmt.state.assign.target.len, stmt.state.assign.target.data);
        printf("value \n");
        print_expression(stmt.state.assign.value);
    }
}

Expression parse_expression(Parser * parser) {
    if(parser->curr().type == TokenType_number_literal) {
        Expression left = {};
        left.type = ExpressionType_literal_number;
        char buffer[1024] = {};
        strncpy(buffer, parser->curr().at.data, parser->curr().at.len);
        buffer[parser->curr().at.len] = 0;
        left.exp.number.value =  atoi(buffer);
        parser->next();
        if (is_binary_operator(parser->curr().type)) {
            Expression parent = {};
            parent.type = ExpressionType_binary;
            parent.exp.binary.left = (Expression *)calloc(1, sizeof(Expression));
            *parent.exp.binary.left = left;
            parent.exp.binary.right = (Expression *)calloc(1, sizeof(Expression));
            parser->next();
            *parent.exp.binary.right = parse_expression(parser);
            return parent;
        }
        return left;
    }
    else if (parser->curr().type == TokenType_string_literal){
        Expression left = {};
        left.type = ExpressionType_literal_string;
        left.exp.string.value = parser->curr().at;
        parser->next();
        if (is_binary_operator(parser->curr().type)){
            Expression parent = {};
            parent.type = ExpressionType_binary;
            parent.exp.binary.left = (Expression *)calloc(1, sizeof(Expression));
            *parent.exp.binary.left = left;
            parent.exp.binary.right = (Expression *)calloc(1, sizeof(Expression));
            parser->next();
            *parent.exp.binary.right = parse_expression(parser);
            return parent;
        }
        return left;
    }
    else if(is_unary_operator(parser->curr().type)) {
        Expression left = {};
        left.type = ExpressionType_unary;
        left.exp.unary.opt.type = get_operator_type(parser->curr().type);
        left.exp.unary.exp = (Expression *) calloc(1, sizeof(Expression));
        parser->next();
        *left.exp.unary.exp = parse_expression(parser);
        if (is_binary_operator(parser->curr().type)) {
            Expression parent = {};
            parent.type = ExpressionType_binary;
            parent.exp.binary.left = (Expression *)calloc(1, sizeof(Expression));
            *parent.exp.binary.left = left;
            parent.exp.binary.right = (Expression *)calloc(1, sizeof(Expression));
            parser->next();
            *parent.exp.binary.right = parse_expression(parser);
            return parent;
        }
        return left;
    }
    else if(parser->curr().type == TokenType_identifier) {
        // not sure if this is required anymore or not 
        if (parser->peek(1).type == TokenType_equal) {
            Expression result = {};
            result.type = ExpressionType_name;
            result.exp.name.value = parser->curr().at;
            parser->next();
            return result;
        }
        // handling all binary operators: 
        // currently does not handle anything like a function call of a group expression
        else if (is_binary_operator(parser->peek(1).type)) {
            Expression result = {};
            result.type = ExpressionType_binary;
            result.exp.binary.left = ( Expression *) calloc(1, sizeof(Expression));
            result.exp.binary.left->type = ExpressionType_name;
            result.exp.binary.left->exp.name.value = parser->curr().at;
            parser->next();
            result.exp.binary.opt.type = get_operator_type(parser->curr().type);
            parser->next();
            result.exp.binary.right = (Expression *) calloc(1, sizeof(Expression));
            *result.exp.binary.right = parse_expression(parser);
            return result;
        }

    }
    if (parser->peek(1).type == TokenType_opening_paran)
    {
        parser->next();
        int group_capacity = 10;
        Expression *expressions = (Expression *)calloc(10, sizeof(Expression));
        int group_size = 0;
        while (parser->curr().type != TokenType_eof && parser->curr().type != TokenType_closing_paran) {
            if (group_size == group_capacity) {
                expressions = (Expression *) realloc(expressions, sizeof (Expression) * 2 * group_capacity);
                group_capacity *= 2;
            }
            expressions[group_size] = parse_expression(parser);
            group_size+=1;
            if (parser->curr().type == TokenType_closing_paran || parser->curr().type == TokenType_comma) {
                parser->next();
            }
        }
        expressions = (Expression *)realloc(expressions, sizeof(Expression) * group_size);
    }
    return {};
}

Statement parse_assignment_statement(Parser * parser) {
    Statement statement;
    statement.type = StatementType_assign;
    statement.state.assign.target = parser->curr().at;
    // skipping the '=' in the process;
    parser->next();
    parser->next();
    statement.state.assign.value = parse_expression(parser);
    return statement;
}

#define MAX_STATEMENT_COUNT 100
Statement * parse_statement(Parser * parser, int * out_statement_count) {
    Statement * statements = (Statement *) malloc(sizeof(Statement) * MAX_STATEMENT_COUNT);
    int statement_idx = 0;
    while (!parser->end() && parser->curr().type != TokenType_eof) {
        if (parser->curr().type == TokenType_identifier) {
            if (parser->peek(1).type == TokenType_equal) {
                statements[statement_idx++] = parse_assignment_statement(parser);
            }
        }
        if (parser->curr().type == TokenType_semi_colon) {
            parser->next();
            continue;
        }
    }
    *out_statement_count = statement_idx;
    return statements;
};

int main(){
    String text = load_entire_file("a.nit");
    printf("source loaded with length : %d", (int)text.len);
    printf("%s", text.data);
    printf("source ---- \n%.*s\n ---- end", (int) text.len, text.data);
    ParsedTokens parsed_token = fetch_tokens(text);
    for(int i = 0 ; i < parsed_token.count; i++) {
        print_token(parsed_token.tokens[i]);
    }
    Parser parser;
    parser.pt = parsed_token;
    parser.idx = 0;
    int statement_count = 0;
    Statement * statements = parse_statement(&parser, &statement_count);
    for(int i = 0;  i < statement_count; i++) {
        printf("statement -----\n");
        print_statement(statements[i]);
    }
    return 0;
}

String load_entire_file(const char * file_path) {
    String result = {};
    FILE * fp = fopen(file_path, "r");
    fseek(fp, 0, SEEK_END);
    result.len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    result.data = (char *)malloc(result.len);
    fread(result.data, result.len, 1, fp);
    return result;
}

void eat_white_space(Cursor * cursor) {
    while(is_white_space(*cursor->at)) {
        cursor->at +=1;
    }
}

void print_token(Token token) {
    printf("type : %d token : %.*s\n", token.type, (int)token.at.len, token.at.data);
}


ParsedTokens fetch_tokens(String file_source) {
    Token * tokens = NULL;
    size_t token_capacity = 0;
    size_t token_count = 0;

    Cursor cursor = {};
    cursor.at = file_source.data;

#define _INTERNAL_CURSOR_AT_EOF() (cursor.at == (file_source.data + file_source.len))
    while( !_INTERNAL_CURSOR_AT_EOF() && *cursor.at != 0) {

        while(!_INTERNAL_CURSOR_AT_EOF() && is_white_space(*cursor.at)) {
            cursor.at += 1;
        }
        if (_INTERNAL_CURSOR_AT_EOF()) break;

        Token token = {};
        // default to a symbole
        token.at.data = cursor.at;
        token.at.len = 1;

        char At = *cursor.at;
        unsigned long long start = cursor.at - file_source.data;

        cursor.at += 1;

        // printf("processing %c \n", At);
        switch(At){
            case 0: token.type = TokenType_eof; break;
            case ',': token.type = TokenType_comma; break;
            case ';': token.type = TokenType_semi_colon; break;
            case '}': token.type = TokenType_closing_brace; break;
            case '{': token.type = TokenType_opening_brace; break;
            case '(': token.type = TokenType_opening_paran; break;
            case ')': token.type = TokenType_closing_paran; break;
            case '!': { // can be bang, but can also be bang_equal
                if (!_INTERNAL_CURSOR_AT_EOF() && *cursor.at == '=')  {
                    token.type = TokenType_bang_equal;
                    token.at.len = 2;
                    cursor.at += 1;
                }
                else {
                    token.type = TokenType_bang;
                }
            } break;
            case '=': {
                if (!_INTERNAL_CURSOR_AT_EOF() && *cursor.at == '=')  {
                    token.type = TokenType_double_equal;
                    token.at.len = 2;
                    cursor.at += 1;
                }
                else {
                    token.type = TokenType_equal;
                }
            } break;
            case '/': token.type = TokneType_forward_slash; break;
            case '*': token.type = TokenType_astricks; break;
            case '-': token.type = TokenType_minus; break;
            case '+': token.type = TokenType_plus; break;
            default:
            {
                if (At >= '0' and At <= '9') {
                    // number literal 
                    while(!_INTERNAL_CURSOR_AT_EOF() && is_number(*cursor.at)) {
                        cursor.at += 1;
                    }
                    if (!is_white_space(*cursor.at) && *cursor.at != ';') {
                        printf("error failed to parse number correctly");
                        exit(1);
                    }
                    token.at.data = (char *)file_source.data + start;
                    token.at.len = (size_t)(cursor.at - file_source.data - start);
                    token.type = TokenType_number_literal;
                }
                else if (At == '"') {

                    while( !_INTERNAL_CURSOR_AT_EOF() && *cursor.at != '"') {
                        if (*cursor.at == '\\') {
                            cursor.at += 1;
                        }
                        cursor.at += 1;
                    }

                    if (*cursor.at != '"') {
                        printf("error failed to parse string correctly");
                        exit(1);
                    }

                    token.at.data = (char *)  file_source.data + start + 1;
                    token.at.len = (size_t)(cursor.at - file_source.data - start - 1);
                    token.type = TokenType_string_literal;

                    cursor.at += 1; // don't handle the same " again.
                }
                else {

                    while(
                        !_INTERNAL_CURSOR_AT_EOF() && (is_alpha(*cursor.at) || is_number(*cursor.at) || *cursor.at == '_')){
                            cursor.at += 1;
                    }

                    token.at.data = (char *) file_source.data  + start ;
                    token.at.len = (size_t)(cursor.at - file_source.data - start);
                    token.type = TokenType_identifier;
#define _INTERNAL_CHECK_TOKEN_MATCH(x) (token.at.len == strlen(x) && strncmp(token.at.data, x, strlen(x)))
                    if (_INTERNAL_CHECK_TOKEN_MATCH("true"))
                        token.type = TokenType_bool_true;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("false"))
                        token.type = TokenType_bool_false;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("if"))
                        token.type = TokenType_if;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("else"))
                        token.type = TokenType_else;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("for"))
                        token.type = TokenType_for;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("return"))
                        token.type = TokenType_return;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("func"))
                        token.type = TokenType_func;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("print"))
                        token.type = TokenType_print;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("and"))
                        token.type = TokenType_and;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("or"))
                        token.type = TokenType_or;
#undef _INTERNAL_CHECK_TOKEN_MATCH
                }
            }break;
        }

        if (token_capacity == 0) {
            tokens = (Token *) malloc(sizeof(Token) * 64);
            token_capacity = 64;
            token_count = 0;
        }
        else if (token_capacity == token_count) {
            tokens = (Token *)realloc(tokens, sizeof(Token) * token_capacity * 2);
            token_capacity = 2 * token_capacity;
        }
        tokens[token_count] = token;
        token_count += 1;
    }
#undef _INTERNAL_CURSOR_AT_EOF

    if (tokens[token_count - 1].type != TokenType_eof) {
        if (token_capacity == 0) {
            tokens = (Token *) malloc(sizeof(Token) * 64);
            token_capacity = 64;
            token_count = 0;
        }
        else if (token_capacity == token_count) {
            tokens = (Token *)realloc(tokens, sizeof(Token) * token_capacity * 2);
            token_capacity = 2 * token_capacity;
        }
        tokens[token_count] = {.type=TokenType_eof, .at = {.data = (char *)"EOF", .len = 3}};
        token_count += 1;
    }

    ParsedTokens result;
    result.tokens = tokens;
    result.count = token_count;

    return result;
}

bool is_white_space(char a) {
    switch (a) {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
        return true;
    default:
        return false;
    }
}

OperatorType get_operator_type(TokenType type) {
    if (type == TokenType_plus) {
        return OperatorType_add;
    } else if (type == TokenType_and) {
        return OperatorType_add;
    } else if (type == TokenType_or) {
        return OperatorType_or;
    } else if (type == TokenType_minus) {
        return OperatorType_subtract;
    } else if (type == TokenType_astricks) {
        return OperatorType_mult;
    } else if (type == TokneType_forward_slash){
        return OperatorType_divide;
    }
    return OperatorType_none;
}
bool is_unary_operator(TokenType type) {
    return (type == TokenType_plus || type == TokenType_minus || type == TokenType_bang || type == TokneType_forward_slash);
}

bool is_binary_operator(TokenType type) {
    return (type == TokenType_plus || type == TokenType_minus || type == TokenType_astricks);
}

bool is_alpha(char a) {
    return (a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z');
}

bool is_number(char a) {
    return a >= '0' && a <= '9';
}