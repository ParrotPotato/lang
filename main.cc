/*
TODO: 
- implementing different 'statement' types (if, else, func & var decl, return, defer, for)
    - assign          // done 
    - if statement    // done
    - block statmeent // done 
    - func decl     
    - var decl 
    - return  // done 
    - defer   // done
- implementing a struct dereference, array and array indexing 
    - array indexing         // done
    - function call refactor // done
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

automatically initialize stuff to be 0 of whatever their type can be
*/

#define PRINT_EXPRESSION
#define PRINT_TOKEN
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
void print_full_token(int idx, Token token);

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
    float unary_power;
    const char * str;
    float left, right;
    OperatorType type;
};

struct NameExp {
    StringView value;
};


struct BoolLiteralExp {
    bool value;
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
    Expression * prefix;
    GroupExp   group;
};

struct IndexExp {
    Expression * prefix;
    Expression * idx_exp;
};

struct Expression {
    ExpressionType type;
    union {
        BinaryExp           binary;
        UniaryExp           unary;
        BoolLiteralExp      boolean;
        NumberLiteralExp    number;
        StringLiteralExp    string;
        GroupExp            group;
        NameExp             name;
        CallExp             call;
        IndexExp            idx;
    };
};

Expression read_only_conv_group_to_expression(GroupExp * gp) {
    Expression result = {};
    result.type = ExpressionType_group;
    result.group = *gp;
    return result;
}
    

void print_full_expression(Expression exp) {

    // printf("type(%s)", print_expression(exp.type));
    if (exp.type == ExpressionType_name) {
        int len = exp.name.value.len;
        char * data = exp.name.value.data;
        printf(" %.*s", len, data);
    }
    else if (exp.type == ExpressionType_literal_boolean) {
        printf(" %s", exp.boolean.value ? "true" : "false");
    }
    else if (exp.type == ExpressionType_literal_number) {
        printf(" %d", exp.number.value);
    }
    else if (exp.type == ExpressionType_unary) {
    printf(" (");
        printf("%s", exp.unary.opt.str);
        print_full_expression(*exp.unary.exp);
    printf(")");
    }
    else if (exp.type == ExpressionType_literal_string) {
        int len = exp.string.value.len;
        char * data = exp.string.value.data;
        printf(" \"%.*s\"", len, data);
    }
    else if (exp.type == ExpressionType_binary) {
    printf(" (");
        printf("%s", exp.binary.opt.str);
        print_full_expression(*exp.binary.left);
        print_full_expression(*exp.binary.right);
    printf(")");
    }
    else if (exp.type == ExpressionType_group) {
        printf(" (");
        for(int i = 0 ; i < exp.group.expression_count ; i++){
            print_full_expression(exp.group.expressions[i]);
            if (i != exp.group.expression_count - 1) {
                printf(",");
            }
        }
        printf(")");
    } else if (exp.type == ExpressionType_call) {
        printf("(");
        print_full_expression(*exp.call.prefix);
        Expression ro_exp = read_only_conv_group_to_expression(&exp.call.group);
        print_full_expression(ro_exp);
        printf(")");
    } else if (exp.type == ExpressionType_index) {
        printf("(");
        printf("idx ");
        print_full_expression(*exp.idx.prefix);
        print_full_expression(*exp.idx.idx_exp);
        printf(")");
    }
}

struct Statement;

struct BlockStat {
    Statement * statements;
    int statement_count;
};

struct AssignStat {
    Expression target; 
    Expression value;
};

struct IfStat {
    Statement * condition_prefix; // for declaring stuff inside the if condition already 
    Expression condition_expression;
    BlockStat if_block;
    BlockStat * else_block;
};

struct ReturnStat {
    Expression exp;
};

struct DeferStat {
    Statement * stmt;
};

struct ExpressionStat {
    Expression exp;
};

struct Statement {
    StatementType type;
    union {
        ExpressionStat exp;
        AssignStat     assign;
        ReturnStat     ret;
        DeferStat      defer;
        IfStat         ifcon; // if statements
        BlockStat      block;
    };
};

void print_full_statement(Statement stmt) {
    if (stmt.type  == StatementType_return) {
        printf("return ");
        if (stmt.ret.exp.type != ExpressionType_none){
            print_full_expression(stmt.ret.exp);
        }
        printf("\n");
    }
    else if (stmt.type  == StatementType_defer) {
        printf("defer ");
        print_full_statement(*stmt.defer.stmt);
        printf("\n");
    }
    else if (stmt.type  == StatementType_assign) {
        print_full_expression(stmt.assign.target);
        printf(" = "); 
        print_full_expression(stmt.assign.value);
        printf("\n");
    }
    else if (stmt.type == StatementType_block) {
        printf(" -- block start -- \n");
        for(int i = 0 ; i < stmt.block.statement_count ; i++) {
            print_full_statement(stmt.block.statements[i]);
        }
        printf(" -- block end -- \n");
    }
    else if (stmt.type == StatementType_if){
        IfStat ifst = stmt.ifcon;
        printf(" -- if start -- \n");
        if (ifst.condition_prefix)  {
            printf("condition:");
            print_full_statement(*ifst.condition_prefix);
            printf("\n");
        }
        printf("if :\n");
        printf("expression: ");
        print_full_expression(ifst.condition_expression);
        printf("\n");
        print_full_statement({.type = StatementType_block, .block = ifst.if_block});
        if (ifst.else_block) {
            printf("else :\n");
            print_full_statement({.type = StatementType_block, .block = *ifst.else_block});
            printf("\n");
        }
        printf(" -- if end -- \n");
    }  else if (stmt.type == StatementType_expression){
        ExpressionStat exp = stmt.exp;
        print_full_expression(exp.exp);
        printf("\n");
    }
}

Expression pratt_parse_expression(Parser * parser, float min_val);

NameExp parse_name_expression(Parser * parser) {
    NameExp name = {};
    name.value = parser->curr().at;
    parser->next();
    return name;
}

GroupExp parse_group_expression(Parser *parser) {
    GroupExp group = {};
    parser->next();
    int group_capacity = 10;
    Expression *expressions = (Expression *)calloc(10, sizeof(Expression));
    int group_size = 0;
    while (parser->curr().type != TokenType_eof && parser->curr().type != TokenType_closing_paran) {
        if (group_size == group_capacity) {
            expressions = (Expression *)realloc(expressions, sizeof(Expression) * 2 * group_capacity);
            group_capacity *= 2;
        }
        expressions[group_size] = pratt_parse_expression(parser, 0);
        group_size += 1;
        if (parser->curr().type == TokenType_comma) {
            parser->next();
        }
    }
    if (parser->curr().type != TokenType_closing_paran) {
        printf("line-no %d: expected ) to close (\n", parser->curr().line_no);
        exit(-1);
    }
    parser->next();
    expressions = (Expression *)realloc(expressions, sizeof(Expression) * group_size);
    group.expression_count = group_size;
    group.expressions = expressions;
    return group;
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


Operator get_operator(TokenType type);
Expression parse_node(Parser * parser) {
    if (parser->curr().type == TokenType_semi_colon) {
        printf("unexpected ; encountered exit(-1)");
        exit(1);
    }
    else if (parser->curr().type == TokenType_bool_false || parser->curr().type == TokenType_bool_true){
        printf("- found boolean lietral\n");
        Expression left = {};
        left.type = ExpressionType_literal_boolean;
        left.boolean.value = parser->curr().type == TokenType_bool_true;
        parser->next();
        return left;
    }
    else if(parser->curr().type == TokenType_number_literal) {
        printf("- found number literal\n");
        Expression left = {};
        left.type = ExpressionType_literal_number;
        left.number = parse_number_literal_expression(parser);
        return left;
    }
    else if (parser->curr().type == TokenType_string_literal){
        printf("- found string literal\n");
        Expression left = {};
        left.type = ExpressionType_literal_string;
        left.string = parse_string_literal_expression(parser);
        return left;
    }
    // on rethinking this,  i think it can only be a number, name, or an group expression (that too a size 0), 
    // anything else here would not make sense, since this is a unary operator 
    else if(is_unary_operator(parser->curr().type)) {
        printf("- found unary operator\n");
        Expression left = {};
        left.type = ExpressionType_unary;
        left.unary.opt = get_operator(parser->curr().type);
        left.unary.exp = (Expression *) calloc(1, sizeof(Expression));
        parser->next();
        *left.unary.exp = pratt_parse_expression(parser, left.unary.opt.unary_power); 
        return left;
    }
    else if (parser->curr().type == TokenType_opening_paran) {
        printf("- found opening paran\n");
        Expression left = {};
        left.type = ExpressionType_group;
        left.group = parse_group_expression(parser); // parse the entire thing, this works perfectly
        return left;
    }
    // NOTE(nitesh: 2026-10-09) : this entire section needs to be re-written because identifier can be a chain of [node].[node][0].[node].function_call().[node]
    else if(parser->curr().type == TokenType_identifier) {
        printf("- found identifier\n");
        // NOTE(nitesh: 2026-10-09): this does not account for member function (methods as people call them) 
        // and dereference type function calls
        // 
        // this is a function call 
        //if (parser->peek(1).type == TokenType_opening_paran) {
        //    printf("- parsing identifier as call expression\n");
        //    CallExp call = parse_call_expression(parser);
        //    Expression exp = {};
        //    exp.type = ExpressionType_call;
        //    exp.exp.call = call;
        //    return exp;
        //}
        printf("- parsing identifier name expression\n");
        Expression left = {};
        left.type = ExpressionType_name;
        left.name = parse_name_expression(parser);
        return left;
    }
    printf("line-no %d: unexpected token %s in expression\n", parser->curr().line_no, print_token(parser->curr().type));
    exit(-1);
    return Expression{};
}

Expression create_binary_expression(Expression * left, Expression * right, Operator op) {
    Expression binary = {};
    binary.type = ExpressionType_binary;
    binary.binary.left = (Expression *)malloc(sizeof(Expression));
    binary.binary.right = (Expression *)malloc(sizeof(Expression));
    *binary.binary.left = *left;
    *binary.binary.right = *right;
    binary.binary.opt =  op;
    return binary;
}

// this ( as operator in my mind only comes in one place can be part of a function call 
// myFunc(x, y); -> function call 
// func myFunc(x : i32, y ; 32) -> void / -> (result : f32) -> function declaration 
Expression parse_opening_paran_as_operator(Parser * parser, Expression left) { // we need to return an expression for function parsing
    // assertion check 
    if (parser->curr().type != TokenType_opening_paran) {
        printf("parse opening paran when current token in not opening_paran i.e. '('\n");
        printf("error on line number %d\n", parser->curr().line_no);
        exit(-1);
    }
    GroupExp group = parse_group_expression(parser);
    CallExp call = {};
    call.prefix= (Expression *)malloc(sizeof(Expression));
    *call.prefix= left;
    call.group = group;
    return Expression{ .type = ExpressionType_call, .call = call};
}

Expression parse_opening_square_bracket_as_operator(Parser * parser, Expression left) {
    if (parser->curr().type != TokenType_opening_square_bracket) {
        printf("parse opening square bracked when current token is not opening square bracket i.e. '['\n");
        printf("error on line number %d\n", parser->curr().line_no);
        exit(-1);
    }
    parser->next();
    Expression idx_exp = pratt_parse_expression(parser, 0);
    if (parser->curr().type != TokenType_closing_square_bracket) {
        printf("expedted ] after [, but cloudn't find it");
        printf("error on line number %d\n", parser->curr().line_no);
        exit(-1);
    }
    parser->next();
    IndexExp idx = {};
    idx.idx_exp = (Expression *)malloc(sizeof(Expression));
    idx.prefix = (Expression *)malloc(sizeof(Expression));
    *idx.idx_exp = idx_exp;
    *idx.prefix = left;
    return Expression{ .type = ExpressionType_index, .idx = idx};
};

Expression pratt_parse_expression(Parser * parser, float min_val) {
    // moves the parser ahead if requried
    Expression left_node = parse_node(parser); 
    while (true) {
        Operator op = get_operator(parser->curr().type);
        if (op.left == 0.0f || min_val > op.left) break;
        if (op.type == OperatorType_opening_paran) {
            left_node = parse_opening_paran_as_operator(parser, left_node);
            continue;
        }
        if (op.type == OperatorType_opening_square_bracket) {
            left_node = parse_opening_square_bracket_as_operator(parser, left_node);
            continue;
        }
        parser->next();
        Expression right_node = pratt_parse_expression(parser, op.right);
        left_node = create_binary_expression(&left_node, &right_node, op);
    }
    return left_node;
}

Statement parse_statement(Parser * parser);

ReturnStat parse_return_statment(Parser * parser){
    ReturnStat ret = {};
    parser->next();
    if (parser->curr().type != TokenType_semi_colon) ret.exp = pratt_parse_expression(parser, 0.0f);
    return ret;
}

AssignStat parse_assignment_statement(Parser * parser, Expression left_prefix) {
    if (parser->curr().type != TokenType_equal) {
        printf("line %d; assignment operator expects parsing = after node expression\n", parser->curr().line_no);
        exit(-1);
    }
    parser->next();
    printf("[] parsing assignment statment\n");
    AssignStat ass = {};
    ass.target = left_prefix;
    ass.value = pratt_parse_expression(parser, 0.0);
    return ass;
}

BlockStat parse_block_statment (Parser * parser) { // tested
    BlockStat block = {};
    int stm_cap  = 10;
    int stm_size = 0;
    Statement * stms = (Statement *) calloc(stm_cap, sizeof(Statement));
    printf("> parsing block statmeent start\n");
    if (parser->curr().type == TokenType_opening_brace) { 
        printf("> consuming opening brace\n");
        parser->next(); // consuming the opening brace
        while (parser->curr().type != TokenType_eof && parser->curr().type != TokenType_closing_brace) {
            Statement stm= parse_statement(parser); // parsing all the statements till we encounter a closing brace
            if (stm_cap == stm_size){
                stms = (Statement *) realloc(stms, sizeof(Statement) * 2 * stm_cap);
                stm_cap = 2 * stm_cap;
            }
            stms[stm_size] = stm;
            stm_size+=1;
            while(parser->curr().type == TokenType_semi_colon) parser->next();
        }
        printf("block statment: %s token %.*s value \n", print_token(parser->curr().type), (int)parser->curr().at.len, parser->curr().at.data);
        if (parser->curr().type == TokenType_eof) {
            printf("closing bracked not found for block statement");
            exit(1);
        }
        printf("> consuming closing brace\n");
        parser->next();  // consuming the closing brace
    } else {
        printf("token : %s value : %.*s\n",
               print_token(parser->curr().type),
               (int)parser->curr().at.len,
               parser->curr().at.data);
        printf("failed to find opening brace for block expression\n");
        exit(1);
    }
    printf("> parsing block statmeent ends\n");
    stms = (Statement *) realloc(stms, sizeof(Statement) * stm_size);
    block.statements = stms;
    block.statement_count = stm_size;
    return block;
}

IfStat parse_if_statement (Parser * parser) {
    printf("- starting parsing if statement\n");
    IfStat ifsmt = {};
    parser->next();
    int index = 0;
    printf("- peeking forward\n");
    while(parser->peek(index).type != TokenType_semi_colon && parser->peek(index).type != TokenType_opening_brace) index++;
    if (parser->peek(index).type == TokenType_semi_colon) {
        printf("- found condition prefix\n");
        Statement smt = parse_statement(parser);
        ifsmt.condition_prefix = (Statement *) calloc(1, sizeof(smt));
        *ifsmt.condition_prefix = smt;
        parser->next(); 
    }  else {
        printf("- no condition prefix found\n");
    }
    printf("- parsing conditional expression\n");
    Expression conditional = pratt_parse_expression(parser, 0.0); 
    ifsmt.condition_expression = conditional;
    if (parser->curr().type != TokenType_opening_brace) {
        printf("line-no %d: opening brace not found after if condition parsing", parser->curr().line_no);
        exit(1);
    }
    printf("- parsing block statement\n");
    ifsmt.if_block = parse_block_statment(parser);
    if (parser->curr().type == TokenType_else) {
        parser->next();
        printf("- parsing else block\n");
        if (parser->curr().type == TokenType_if) {
            IfStat elif = parse_if_statement(parser);
            BlockStat block = {};
            block.statement_count = 1;
            block.statements = (Statement *) calloc(1, sizeof(Statement));
            block.statements[0].type = StatementType_if;
            block.statements[0].ifcon = elif ;
            ifsmt.else_block = (BlockStat *) calloc(1, sizeof(BlockStat));
            *ifsmt.else_block = block;
        } else {
            if (parser->curr().type != TokenType_opening_brace) {
                printf("line-no %d: opening brace not found after else phrase", parser->curr().line_no);
                exit(1);
            }
            BlockStat block = parse_block_statment(parser);
            ifsmt.else_block = (BlockStat *) calloc(1, sizeof(BlockStat));
            *ifsmt.else_block = block;
        }
    } else {
        printf("- no else block found\n");

    }
    return ifsmt;
}

Statement parse_statement(Parser * parser) {

    printf("[line: %d] starting statmeent parsing with token : %s\n", parser->curr().line_no, print_token(parser->curr().type));
    printf("[line: %d] text string %.*s\n", parser->curr().line_no, (int)parser->curr().at.len, (char *)parser->curr().at.data);

    Statement ret = {};

    // we don't care about handling this situation
    if (parser->curr().type == TokenType_semi_colon) {
        parser->next();
        ret.type = StatementType_none;
        return ret;
    }
    else if (parser->curr().type == TokenType_return) {
        ReturnStat exp = parse_return_statment(parser);
        ret.type = StatementType_return;
        ret.ret = exp;
        return ret;
    }
    else if (parser->curr().type == TokenType_defer) {
        if (parser->peek(1).type == TokenType_semi_colon) {
            printf("line-no %d: defer should be followed by a non empty statemetn\n", parser->curr().line_no);
            exit(-1);
        }
        parser->next();
        Statement stmt = parse_statement(parser);
        ret.type = StatementType_defer;
        ret.defer.stmt = (Statement *) calloc(1, sizeof(Statement));
        *ret.defer.stmt = stmt;
        return ret;
    }
    else if (parser->curr().type == TokenType_if) {
        IfStat ifstat = parse_if_statement(parser);
        ret.type = StatementType_if;
        ret.ifcon = ifstat;
        return ret;
    } else if (parser->curr().type == TokenType_opening_brace) {
        BlockStat block = parse_block_statment(parser);
        ret.type = StatementType_block;
        ret.block = block;
        return ret;
    }
    else {
        // we are not parsing something with start with an expresion 
        // like a functionc all foo();
        // or an assignment operator x = y + d3;
        Expression left_prefix = pratt_parse_expression(parser, 0.0f);

        if (parser->curr().type == TokenType_equal) {
            AssignStat ass = parse_assignment_statement(parser, left_prefix);
            ret.type = StatementType_assign;
            ret.assign = ass;
            return ret;
        } else { 
            // this is a stand along statemenet which is a single expression
            ExpressionStat exp = {.exp = left_prefix};
            ret.exp = exp;
            ret.type = StatementType_expression;
            return ret;
        }
    }
    return {};
}

Statement * parse_statements(Parser * parser, int * out_statement_count) {
    int stm_cap = 10;
    Statement * stms = (Statement *) malloc(sizeof(Statement) * stm_cap);
    int stm_idx = 0;
    while (!parser->end() && parser->curr().type != TokenType_eof) {
        Statement stm= parse_statement(parser);
        if (stm_idx == stm_cap){
            stms = (Statement *) realloc(stms, sizeof(Statement) * stm_cap * 2);
            stm_cap *= 2;
        }
        stms[stm_idx] = stm;
        stm_idx+=1;
    }
    *out_statement_count = stm_idx;
    int idx = 0;
    while (idx!=stm_idx){
        printf("printing statmeent idx[%d]\n", idx);
        print_full_statement(stms[idx]);
        idx += 1;
    }
    return stms;
};

int main(){
    String text = load_entire_file("b.nit");
    ParsedTokens parsed_token = fetch_tokens(text);
#ifdef PRINT_TOKEN
    for(int i = 0 ; i < parsed_token.count; i++) {
        print_full_token(i, parsed_token.tokens[i]);
    }
#endif
    Parser parser;
    parser.pt = parsed_token;
    parser.idx = 0;
    int statement_count = 0;
    Statement * statements = parse_statements(&parser, &statement_count);
    printf("%.*s\n", (int)text.len, text.data);
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

void print_full_token(int idx, Token token) {
    printf("index : %d, line_no: %d type : %d token : %.*s\n", idx, token.line_no, token.type, (int)token.at.len, token.at.data);
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

        bool was_comment = false;
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
            case '/': {
                if (!_INTERNAL_CURSOR_AT_EOF() && *cursor.at == '/')  {
                    // this is a single line comment now, please skip till the \n
                    while(*cursor.at != '\n') {
                        cursor.at += 1;
                        token.at.len += 1;
                    }
                    was_comment = true;
                }
                else {
                    token.type = TokenType_forward_slash;
                }
            } break;
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
                    if (!is_white_space(*cursor.at) && *cursor.at != ';' && *cursor.at != ',' && *cursor.at != ')' && *cursor.at != ']') {
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
#define _INTERNAL_CHECK_TOKEN_MATCH(x) (token.at.len == strlen(x) && (strncmp(token.at.data, x, strlen(x)) == 0))
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
                        token.type = TokenType_defer;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("continue"))
                        token.type = TokenType_continue;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("break"))
                        token.type = TokenType_break;
                    else if (_INTERNAL_CHECK_TOKEN_MATCH("switch"))
                        token.type = TokenType_switch;
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
        if (!was_comment) {
            tokens[token_count] = token;
            token_count += 1;
        }

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

Operator get_operator(TokenType type) {
    if (type == TokenType_astricks) {                      return Operator{ .unary_power =  0, .str="*",     .left = 6.0f,   .right = 6.1f,  .type = OperatorType_mult, };
    } else if (type == TokenType_forward_slash){           return Operator{ .unary_power =  0, .str="/",     .left = 6.0f,   .right = 6.1f,  .type = OperatorType_divide, };
    } else if (type == TokenType_dot) {                    return Operator{ .unary_power =  0, .str = ".",   .left = 20.0f,  .right = 20.1f, .type = OperatorType_subtract, };
    } else if (type == TokenType_plus) {                   return Operator{ .unary_power =  0, .str = "+",   .left = 5.0f,   .right = 5.1f,  .type = OperatorType_add, };
    } else if (type == TokenType_minus) {                  return Operator{ .unary_power = 15, .str = "-",   .left = 5.0f,   .right = 5.1f,  .type = OperatorType_subtract, };
    } else if (type == TokenType_double_equal){            return Operator{ .unary_power =  0, .str="==",    .left = 4.0f,   .right = 4.1f,  .type = OperatorType_equal, };
    } else if (type == TokenType_bang) {                   return Operator{ .unary_power = 15, .str="!",     .left = 4.0f,   .right = 4.1f,  .type = OperatorType_not_equal, };
    } else if (type == TokenType_bang_equal){              return Operator{ .unary_power =  0, .str="!=",    .left = 4.0f,   .right = 4.1f,  .type = OperatorType_not_equal, };
    } else if (type == TokenType_and) {                    return Operator{ .unary_power =  0, .str = "and", .left = 3.0f,   .right = 3.1f,  .type = OperatorType_and, };
    } else if (type == TokenType_or) {                     return Operator{ .unary_power =  0, .str = "or",  .left = 2.0f,   .right = 2.1f,  .type = OperatorType_or, };
    } else if (type == TokenType_opening_paran){           return Operator{ .unary_power =  0, .str="(",     .left = 20.0f,  .right = 0.0f,  .type = OperatorType_opening_paran};
    } else if (type == TokenType_closing_paran){           return Operator{ .unary_power =  0, .str=")",     .left = 0.0f,   .right = 0.0f,  .type = OperatorType_closing_paran};
    } else if (type == TokenType_opening_angle_bracket) {  return Operator{ .unary_power =  0, .str="<",     .left = 4.5f,   .right = 4.6f,  .type = OperatorType_lesser, };
    } else if (type == TokenType_closing_angle_bracket) {  return Operator{ .unary_power =  0, .str=">",     .left = 4.5f,   .right = 4.6f,  .type = OperatorType_greater, };
    } else if (type == TokenType_less_than_equal) {        return Operator{ .unary_power =  0, .str="<=",    .left = 4.5f,   .right = 4.6f,  .type = OperatorType_lesser_equal, };
    } else if (type == TokenType_greater_than_equal) {     return Operator{ .unary_power =  0, .str=">=",    .left = 4.5f,   .right = 4.6f,  .type = OperatorType_greater_equal, };
    } else if (type == TokenType_opening_square_bracket) { return Operator{ .unary_power =  0, .str="[",     .left = 20.0f,  .right = 0.0f,  .type = OperatorType_opening_square_bracket};
    } else if (type == TokenType_closing_square_bracket) { return Operator{ .unary_power =  0, .str="]",     .left = 0.0f,   .right = 0.0f,  .type = OperatorType_closing_square_bracket};
    } else {                                     return Operator{ .unary_power =  0, .str="not_operator", .left = 0.0f, .right = 0.0f, .type = OperatorType_none};
    }
}

bool is_unary_operator(TokenType type) {
    return (
        type == TokenType_plus || 
        type == TokenType_minus || 
        type == TokenType_bang || 
        type == TokenType_forward_slash || 
        type == TokenType_dot
    );

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
