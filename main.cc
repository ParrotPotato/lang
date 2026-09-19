/*
TODO: 
- implementing different 'statement' types (if, else, func & var decl, return, defer, for)
    - assign // done 
    - if statement 
    - func decl 
    - var decl 
    - return  // done 
    - defer
- implementing a struct dereference, array and array indexing 
- define language grammar 
    - how are things declared
    - how are thiings asisgned 
    - what are the semantic rules around what should be done when, specifically, around 
        - multiple returns 
        - loop definitions
        - scope and variable shadowing 
        - in scope structs and function declaration
        - (maybe if we are really competent at doing the above tasks): think about how macro's can work, i really like how powerful the macros in C / C++ are - see what can be archived when we do that type of thing
- define type speficitaion for the language 
  int's flot's strings -> are they c like are they not c like, maps, tables etc etc)
- implementing a lua like interepreter which can then allow our programming language to call function 
  and refer to data structurs from out cpp file 
*/

#include <csetjmp>
#define PRINT_EXPRESSION
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "parser.hh"

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
bool is_binary_operator(TokenType type);
bool is_unary_operator(TokenType type);

struct Token {
    TokenType  type;
    StringView at;
    int        line_no;
};
bool is_alpha(char a);
bool is_number(char a);
bool is_white_space(char a);
void print_full_token(Token token);

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

struct GroupExp {
    Expression * expressions;
    int expression_count;
};

struct CallExp {
    StringView call_name;
    GroupExp   group;
};

struct Expression {
    ExpressionType type;
    union {
        BinaryExp           binary;
        UniaryExp           unary;
        NumberLiteralExp    number;
        StringLiteralExp    string;
        GroupExp            group;
        NameExp             name;
        CallExp             call;
    } exp ;
};

Expression read_only_conv_group_to_expression(GroupExp * gp) {
    Expression result;
    result.type = ExpressionType_group;
    result.exp.group = *gp;
    return result;
}
    

void print_full_expression(Expression exp) {

    printf(" (");

    printf("type(%s)", print_expression(exp.type));
    if (exp.type == ExpressionType_name) {
        int len = exp.exp.name.value.len;
        char * data = exp.exp.name.value.data;
        printf(" name(%.*s)", len, data);
    }
    else if (exp.type == ExpressionType_literal_number) {
        printf(" num_lit(%d)", exp.exp.number.value);
    }
    else if (exp.type == ExpressionType_unary) {
        printf(" op(%s)", print_operator(exp.exp.unary.opt.type));
        print_full_expression(*exp.exp.unary.exp);
    }
    else if (exp.type == ExpressionType_literal_string) {
        int len = exp.exp.string.value.len;
        char * data = exp.exp.string.value.data;
        printf(" str_lit(\"%.*s\")", len, data);
    }
    else if (exp.type == ExpressionType_binary) {
        printf(" op(%s)", print_operator(exp.exp.binary.opt.type));
        print_full_expression(*exp.exp.binary.left);
        print_full_expression(*exp.exp.binary.right);
    }
    else if (exp.type == ExpressionType_group) {
        printf(" (");
        for(int i = 0 ; i < exp.exp.group.expression_count ; i++){
            print_full_expression(exp.exp.group.expressions[i]);
            if (i != exp.exp.group.expression_count - 1) {
                printf(",");
            }
        }
        printf(")");
    } else if (exp.type == ExpressionType_call) {
        int len = exp.exp.call.call_name.len;
        char * data = exp.exp.call.call_name.data;
        printf(" call_name(%.*s)", len, data); 
        Expression ro_exp = read_only_conv_group_to_expression(&exp.exp.call.group);
        print_full_expression(ro_exp);
    }

    printf(")");
}

struct Statement;

struct BlockStat {
    Statement * statements;
    int statement_count;
};

struct AssignStat {
    StringView target; 
    Expression value;
};

struct IfStat {
    Statement * condition_prefix; // for declaring stuff inside the if condition already 
    Expression condition_expression;
    BlockStat if_block;
    BlockStat else_block;
};

struct ReturnStat {
    Expression exp;
};

struct Statement {
    StatementType type;
    union {
        AssignStat assign;
        ReturnStat ret;
        IfStat     ifcon; // if condition
    } smt ;
};

void print_statement(Statement stmt) {
    if (stmt.type  == StatementType_assign) {
        int len = stmt.smt.assign.target.len;
        char * data = stmt.smt.assign.target.data;
        printf("%.*s = ", len, data); print_full_expression(stmt.smt.assign.value);
        printf("\n");
    }
}
Expression parse_expression(Parser * parser);

NameExp parse_name_expression(Parser * parser) {
    NameExp name;
    name.value = parser->curr().at;
    parser->next();
    return name;
}

GroupExp parse_group_expression(Parser *parser) {
    GroupExp group;
    parser->next();
    int group_capacity = 10;
    Expression *expressions = (Expression *)calloc(10, sizeof(Expression));
    int group_size = 0;
    while (parser->curr().type != TokenType_eof && parser->curr().type != TokenType_closing_paran) {
        if (group_size == group_capacity) {
            expressions = (Expression *)realloc(expressions, sizeof(Expression) * 2 * group_capacity);
            group_capacity *= 2;
        }
        expressions[group_size] = parse_expression(parser);
        group_size += 1;
        if (parser->curr().type == TokenType_comma) {
            parser->next();
        }
    }
    if (parser->curr().type == TokenType_closing_paran) {
        parser->next();
    }
    expressions = (Expression *)realloc(expressions, sizeof(Expression) * group_size);
    group.expression_count = group_size;
    group.expressions = expressions;
    return group;
}

Expression parse_right_side_of_binary_expression(Parser *parser, Expression left) {
    Expression parent = {};
    parent.type = ExpressionType_binary;
    parent.exp.binary.opt.type = get_operator_type(parser->curr().type);
    parent.exp.binary.left = (Expression *)calloc(1, sizeof(Expression));
    *parent.exp.binary.left = left;
    parent.exp.binary.right = (Expression *)calloc(1, sizeof(Expression));
    parser->next();
    *parent.exp.binary.right = parse_expression(parser);
    return parent;
}

NumberLiteralExp parse_number_literal_expression(Parser *parser) {
    NumberLiteralExp exp = {};
    char buffer[1024] = {};
    strncpy(buffer, parser->curr().at.data, parser->curr().at.len);
    buffer[parser->curr().at.len] = 0;
    exp.value = atoi(buffer);
    parser->next();
    return exp;
}

StringLiteralExp parse_string_literal_expression(Parser * parser) {
    StringLiteralExp exp = {};
    exp.value = parser->curr().at;
    parser->next();
    return exp;
}

CallExp parse_call_expression(Parser * parser) {
    CallExp exp = {}; 
    exp.call_name = parser->curr().at;
    parser->next();
    exp.group = parse_group_expression(parser);
    return exp;
}

// never consume ;  that should be consumed at the statement level 
Expression parse_expression(Parser * parser) {
    if (parser->curr().type == TokenType_semi_colon) return {};
    if(parser->curr().type == TokenType_number_literal) {
        Expression left = {};
        left.type = ExpressionType_literal_number;
        left.exp.number = parse_number_literal_expression(parser);
        if (is_binary_operator(parser->curr().type)) {
            return parse_right_side_of_binary_expression(parser, left);
        }
        return left;
    }
    else if (parser->curr().type == TokenType_string_literal){
        Expression left = {};
        left.type = ExpressionType_literal_string;
        left.exp.string = parse_string_literal_expression(parser);
        if (is_binary_operator(parser->curr().type)){
            return parse_right_side_of_binary_expression(parser, left);
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
            return parse_right_side_of_binary_expression(parser, left);
        }
        return left;
    }
    else if (parser->curr().type == TokenType_opening_paran) {
        Expression left = {};
        left.type = ExpressionType_group;
        left.exp.group = parse_group_expression(parser);
        if (is_binary_operator(parser->curr().type)) {
            return parse_right_side_of_binary_expression(parser, left);
        }
        return left;
    }
    else if(parser->curr().type == TokenType_identifier) {
        // this is a function call
        if (parser->peek(1).type == TokenType_opening_paran) {
            CallExp call = parse_call_expression(parser);
            Expression exp = {};
            exp.type = ExpressionType_call;
            exp.exp.call = call;
            if (is_binary_operator(parser->curr().type)) {
                return parse_right_side_of_binary_expression(parser, exp);
            }
            return exp;
        }
        else {
            Expression left = {};
            left.type = ExpressionType_name;
            left.exp.name = parse_name_expression(parser);
            if (is_binary_operator(parser->curr().type)) {
                return parse_right_side_of_binary_expression(parser, left);
            }
            return left;
        }
    }
    return {};
}

Statement parse_statement(Parser * parser);

Statement parse_return_statment(Parser * parser){
    Statement smt;
    smt.type = StatementType_return;
    parser->next();
    Expression exp = parse_expression(parser);
    smt.smt.ret.exp = parse_expression(parser);
    return smt;
}

Statement parse_assignment_statement(Parser * parser) {
    Statement statement;
    statement.type = StatementType_assign;
    statement.smt.assign.target = parser->curr().at;
    parser->next();
    parser->next();
    statement.smt.assign.value = parse_expression(parser);
    return statement;
}

Statement parse_block_statment (Parser * parser) {

    int smt_capacity = 10;
    int smt_size = 0;
    Statement * smts = (Statement *) calloc(10, sizeof(Statement));
    if (parser->curr().type == TokenType_opening_brace) {
        while (parser->curr().type != TokenType_closing_brace) {
            parser->next();
            Statement statment = parse_statement(parser);
            if (smt_size == smt_capacity) {
                smts = (Statement *) realloc(smts, sizeof(statment) * 2 * smt_capacity);
                smt_capacity *= 2;
            }
            smts[smt_size] = statment;
            smt_size+=1;
        }
        parser->next();
    }
    smts = (Statement *) realloc(smts, sizeof(Statement) * smt_size);
}

Statement parse_if_statement (Parser * parser) {
    Statement smt;

    parser->next();
    int index = 0;
    while(parser->peek(index).type != TokenType_semi_colon && parser->peek(index).type != TokenType_opening_brace) index++;

    if (parser->peek(index).type == TokenType_semi_colon) {
        Statement smt = parse_statement(parser);
    }
    Expression conditional = parse_expression(parser);

    // dodo nitesh: add parsing logic for block else and else block statments 

    smt.type = StatementType_if;
    smt.smt.ifcon.condition_prefix = (Statement *) calloc(1, sizeof(smt));
    smt.smt.ifcon.condition_expression = conditional;

    return smt;
}

Statement parse_statement(Parser * parser) {
    while (parser->curr().type == TokenType_semi_colon) parser->next();

    if (parser->curr().type == TokenType_identifier) {
        if (parser->peek(1).type == TokenType_equal) {
            return parse_assignment_statement(parser);
        }
    } 
    else if (parser->curr().type == TokenType_return) {
        return parse_return_statment(parser);
    }
}

#define MAX_STATEMENT_COUNT 100
Statement * parse_statements(Parser * parser, int * out_statement_count) {
    Statement * statements = (Statement *) malloc(sizeof(Statement) * MAX_STATEMENT_COUNT);
    int statement_idx = 0;
    while (!parser->end() && parser->curr().type != TokenType_eof) {
        Statement statment = parse_statement(parser);
        print_statement(statment);
    }
    *out_statement_count = statement_idx;
    return statements;
};

int main(){
    String text = load_entire_file("a.nit");
    ParsedTokens parsed_token = fetch_tokens(text);
#ifdef PRINT_TOKEN
    for(int i = 0 ; i < parsed_token.count; i++) {
        print_full_token(parsed_token.tokens[i]);
    }
#endif
    Parser parser;
    parser.pt = parsed_token;
    parser.idx = 0;
    int statement_count = 0;
    Statement * statements = parse_statements(&parser, &statement_count);
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

void print_full_token(Token token) {
    printf("line_no: %d type : %d token : %.*s\n", token.line_no, token.type, (int)token.at.len, token.at.data);
}

ParsedTokens fetch_tokens(String file_source) {
    Token * tokens = NULL;
    size_t token_capacity = 0;
    size_t token_count = 0;

    Cursor cursor = {};
    cursor.at = file_source.data;

    int line_number = 0;

#define _INTERNAL_CURSOR_AT_EOF() (cursor.at == (file_source.data + file_source.len))
    while( !_INTERNAL_CURSOR_AT_EOF() && *cursor.at != 0) {

        while(!_INTERNAL_CURSOR_AT_EOF() && is_white_space(*cursor.at)) {
            if (*cursor.at == '\n') {
               line_number+=1; 
            }
            cursor.at += 1;
        }
        if (_INTERNAL_CURSOR_AT_EOF()) break;

        Token token = {};

        token.at.data = cursor.at;
        token.at.len = 1;
        token.line_no = line_number;

        char At = *cursor.at;
        unsigned long long start = cursor.at - file_source.data;

        cursor.at += 1;

        switch(At){
            case 0: token.type = TokenType_eof; break;
            case '[': token.type = TokenType_opening_square_bracket;break;
            case ']': token.type = TokenType_closing_square_bracket;break;
            case '<': 
            {
                if (!_INTERNAL_CURSOR_AT_EOF() && *cursor.at == '=') {
                    token.type = TokenType_less_than_equal;
                    token.at.len = 2;
                    cursor.at += 1;
                }
                else {
                    token.type = TokenType_opening_angle_bracket;
                }
            }; break;
            case '>': 
            {
                if (!_INTERNAL_CURSOR_AT_EOF() && *cursor.at == '=') {
                    token.type = TokenType_greater_than_equal;
                    token.at.len = 2;
                    cursor.at += 1;
                }
                else {
                    token.type = TokenType_closing_angle_bracket;
                }
            }; break;
            
            case '.': token.type = TokenType_dot; break;
            case ',': token.type = TokenType_comma; break;
            case ':': token.type = TokenType_comma; break;
            case ';': token.type = TokenType_semi_colon; break;
            case '}': token.type = TokenType_closing_brace; break;
            case '{': token.type = TokenType_opening_brace; break;
            case '(': token.type = TokenType_opening_paran; break;
            case ')': token.type = TokenType_closing_paran; break;
            case '!': {
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
            case '/': token.type = TokenType_forward_slash; break;
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
                    if (!is_white_space(*cursor.at) && *cursor.at != ';' && *cursor.at != ',' && *cursor.at != ')') {
                        printf("cursor.at %c\n", *cursor.at);
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
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("defer"))
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
        return OperatorType_and;
    } else if (type == TokenType_or) {
        return OperatorType_or;
    } else if (type == TokenType_minus) {
        return OperatorType_subtract;
    } else if (type == TokenType_astricks) {
        return OperatorType_mult;
    } else if (type == TokenType_forward_slash){
        return OperatorType_divide;
    } else if (type == TokenType_double_equal){
        return OperatorType_equal;
    } else if (type == TokenType_bang_equal){
        return OperatorType_not_equal;
    }
    return OperatorType_none;
}
bool is_unary_operator(TokenType type) {
    return (type == TokenType_plus || type == TokenType_minus || type == TokenType_bang || type == TokenType_forward_slash);
}

// NOTE(nitesh): check which enums are missing here
bool is_binary_operator(TokenType type) {
    switch (type){
        case TokenType_plus:
        case TokenType_minus:
        case TokenType_astricks:
        case TokenType_forward_slash:
        case TokenType_and:
        case TokenType_or:
        case TokenType_equal:
        case TokenType_double_equal:
        case TokenType_bang_equal:
        case TokenType_opening_angle_bracket: // less than 
        case TokenType_closing_angle_bracket: // greater than 
        case TokenType_less_than_equal:
        case TokenType_greater_than_equal:
            return true;
    }
    return false;
}

bool is_alpha(char a) {
    return (a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z');
}

bool is_number(char a) {
    return a >= '0' && a <= '9';
}
