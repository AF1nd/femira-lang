#pragma once

#include <string>
#include <vector>
#include <map>
#include <regex>

using namespace std;

enum TokenType 
{
    INT_TYPE,
    NULL_LITERAL,
    STRING_TYPE,
    DOUBLE_TYPE,

    IDENTIFIER,
    STRING,
    DIGIT,

    PLUS,
    MINUS,
    ASTERISK,
    SLASH,
    ASSIGN,

    EQ,
    NOTEQ,

    BIGGER,
    SMALLER,

    BIGGER_OR_EQ,
    SMALLER_OR_EQ,
    
    BEGIN,
    END,

    FUNCTION,

    LPAREN,
    RPAREN,

    COMMA,
    DOT,
    ARROW,

    RETURN,
    PRINT,

    WHITESPACE,
};

struct Token 
{
    TokenType type;
    string value;
    int position;

    Token(TokenType type, string value, int position) { this->position = position; this->value = value; this->type = type; };
};

class Lexer 
{
    private:
        vector<Token*> tokens;
        string code;
        bool trace;
        int position;

        Token* next_token();
    public:

        Lexer(string code, bool trace);

        vector<Token*> make_tokens();
};