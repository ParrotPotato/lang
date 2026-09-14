#pragma once

#define _INTERNAL_MACRO_CONCATENATE(A,B) A##B
#define CONCAT(A,B) _INTERNAL_MACRO_CONCATENATE(A,B)

#define TOKEN_LIST(MACRO_FUNCTION) \
    MACRO_FUNCTION(none)\
    MACRO_FUNCTION(comma)\
    MACRO_FUNCTION(opening_paran)\
    MACRO_FUNCTION(closing_paran)\
    MACRO_FUNCTION(opening_brace)\
    MACRO_FUNCTION(closing_brace)\
    MACRO_FUNCTION(plus)\
    MACRO_FUNCTION(minus)\
    MACRO_FUNCTION(astricks)\
    MACRO_FUNCTION(forward_slash)\
    MACRO_FUNCTION(equal)\
    MACRO_FUNCTION(double_equal)\
    MACRO_FUNCTION(bang)\
    MACRO_FUNCTION(bang_equal)\
    MACRO_FUNCTION(double_quote)\
    MACRO_FUNCTION(semi_colon)\
    MACRO_FUNCTION(bool_true)\
    MACRO_FUNCTION(bool_false)\
    MACRO_FUNCTION(if)\
    MACRO_FUNCTION(else)\
    MACRO_FUNCTION(for)\
    MACRO_FUNCTION(return)\
    MACRO_FUNCTION(func)\
    MACRO_FUNCTION(print)\
    MACRO_FUNCTION(and)\
    MACRO_FUNCTION(or)\
    MACRO_FUNCTION(defer)\
    MACRO_FUNCTION(identifier)\
    MACRO_FUNCTION(string_literal)\
    MACRO_FUNCTION(number_literal)\
    MACRO_FUNCTION(eof)\
    MACRO_FUNCTION(count)

#define OPERATOR_LIST(MACRO_FUNCTION) \
    MACRO_FUNCTION(none)\
    MACRO_FUNCTION(add)\
    MACRO_FUNCTION(subtract)\
    MACRO_FUNCTION(mult)\
    MACRO_FUNCTION(divide)\
    MACRO_FUNCTION(and)\
    MACRO_FUNCTION(or)\
    MACRO_FUNCTION(equal)\
    MACRO_FUNCTION(not_equal)\
    MACRO_FUNCTION(count)

#define EXPRESSION_LIST(MACRO_FUNCTION)\
    MACRO_FUNCTION(none)\
    MACRO_FUNCTION(literal_number)\
    MACRO_FUNCTION(literal_string)\
    MACRO_FUNCTION(name)\
    MACRO_FUNCTION(binary)\
    MACRO_FUNCTION(unary)\
    MACRO_FUNCTION(call)\
    MACRO_FUNCTION(group)\
    MACRO_FUNCTION(count)

#define STATEMENT_LIST(MACRO_FUNCTION)\
    MACRO_FUNCTION(none)\
    MACRO_FUNCTION(assign)\
    MACRO_FUNCTION(if)\
    MACRO_FUNCTION(count)

enum TokenType {
#define TOKEN_ENUMERATE(x) TokenType_##x,
TOKEN_LIST(TOKEN_ENUMERATE)
#undef TOKEN_ENUMERATE
};

enum OperatorType {
#define OPERATOR_ENUMERATE(x) OperatorType_##x,
OPERATOR_LIST(OPERATOR_ENUMERATE)
#undef OPERATOR_ENUMERATE
};

enum ExpressionType {
#define EXPRESSION_ENUMERATE(x) ExpressionType_##x,
EXPRESSION_LIST(EXPRESSION_ENUMERATE)
#undef EXPRESSION_ENUMERATE
};

enum StatementType {
#define STATEMENT_ENUMERATE(x) StatementType_##x,
STATEMENT_LIST(STATEMENT_ENUMERATE)
#undef STATEMENT_ENUMERATE
};

#define PRINT_MACRO(name, Type, CASE, ENUMERATED_LIST) \
const char * print_##name(Type tt) {\
    switch(tt) {\
        ENUMERATED_LIST(CASE)\
        default:\
            return "undefined_" #name;\
    }\
}

#define TOKEN_CASE(x) case CONCAT(TokenType_, x): return #x;
PRINT_MACRO(token, TokenType, TOKEN_CASE, TOKEN_LIST)
#undef TOKEN_CASE

#define OPERATOR_CASE(x) case CONCAT(OperatorType_, x): return #x;
PRINT_MACRO(operator, OperatorType, OPERATOR_CASE, OPERATOR_LIST)
#undef OPERATOR_CASE

#define EXPRESSION_CASE(x) case CONCAT(ExpressionType_, x): return #x;
PRINT_MACRO(expression, ExpressionType, EXPRESSION_CASE, EXPRESSION_LIST)
#undef EXPRESSION_CASE

#define STATEMENT_CASE(x) case CONCAT(StatementType_, x): return #x;
PRINT_MACRO(statement, StatementType, STATEMENT_CASE, STATEMENT_LIST)
#undef STATEMENT_CASE
