#include <iostream>
#include <vector>
#include <map>
#include <array>

#include "include/lexer.h"
#include "include/parser.h"

using namespace std;  

map<TokenType, string> token_types_names = {
    { IDENTIFIER, "identifier" },

    { STRING, "string" },
    { DIGIT, "digit" },
    { NIL, "nil" },
    { TRUE, "true" },
    { FALSE, "false" },

    { PLUS, "+" },
    { MINUS, "-" },
    { ASTERISK, "*" },
    { SLASH, "/" },
    { ASSIGN, ":="},

    { ANNOTATE, ":" },

    { EQ, "?=" },
    { NOTEQ, "!=" },

    { BIGGER, ">" },
    { SMALLER, "<" },

    { BIGGER_OR_EQ, ">=" },
    { SMALLER_OR_EQ, "<=" },
    
    { NOT, "!" },

    { BEGIN, "{" },
    { END, "}" },

    { IF, "if" },
    { ELSE, "else" },

    { WHILE, "while" },
    { FOR, "for" },

    { FUNCTION, "fn" },

    { LPAREN, "(" },
    { RPAREN, ")" },

    { LSQPAREN, "[" },
    { RSQPAREN, "]"},

    { COMMA, "," },
    { DOT, "." },
    { ARROW, "->" },

    { RETURN, "return" },
    { PRINT, "print" },

    { WHITESPACE, "whitespace" },
    { SEMICOLON, "semicolon" },

    { WAIT, "wait" },
};

vector<TokenType> unary_token_types = {
    RETURN,
    PRINT,
    WAIT
};

vector<TokenType> literal_token_types = {
    NIL,
    TRUE,
    FALSE,
    DIGIT,
    STRING,
};

vector<TokenType> binary_token_types = {
    PLUS,
    MINUS,
    ASTERISK,
    SLASH,

    EQ,
    NOTEQ,

    BIGGER,
    SMALLER,

    BIGGER_OR_EQ,
    SMALLER_OR_EQ,

    AND,
    OR,

    ASSIGN,
};

Parser::Parser(vector<Token*> tokens)
{
    this->position = 0;
    this->tokens = tokens;
}

bool Parser::is_token(vector<TokenType> types, int position)
{
    if (position >= this->tokens.size()) return false;

    Token* at_pos = this->tokens.at(position);
    TokenType type = at_pos->type;

    if (find(types.begin(), types.end(), type) != types.end()) return true;
    return false;
}

bool Parser::match(vector<TokenType> types)
{
    if (this->is_token(types, this->position))
    {
        this->position++;
        return true;
    }
    return false;
}

Token* Parser::eat(vector<TokenType> types)
{
    string expected_tokens;
    
    for (TokenType type: types) expected_tokens += token_types_names[type] + " ";

    Token* token = this->tokens.at(this->position);

    if (this->is_token(types, this->position))
    {
        this->position++;
        return token;
    }
    
    this->parser_errorf("Expected token: " + expected_tokens + ", given: " + token_types_names[token->type]);

    return nullptr;
}

void Parser::parser_errorf(string text)
{
    throw runtime_error("Syntax error: " + text + ", at position: " + to_string(this->position));
}

BlockNode* Parser::make_ast(bool trace)
{
    BlockNode* ast = new BlockNode({});

    while (this->position < this->tokens.size())
    {
        AstNode* expression = this->parse_expression();
        if (expression)
        {
            ast->nodes.push_back(expression);
            this->match({ SEMICOLON });
        }
        else break;
    }

    if (trace)
    {
        for (AstNode* node: ast->nodes) cout << node->tostring() << endl;
    }

    return ast;
}

AstNode* Parser::parse_expression(bool ignore_binaries)
{   
    AstNode* expression;

    int started_position = this->position;

    if (this->is_token({ FUNCTION }, this->position)) expression = this->parse_function();
    else if (this->is_token(literal_token_types, this->position)) expression = this->parse_literal();
    else if (this->is_token({ IDENTIFIER }, this->position)) expression = this->parse_identifier();
    else if (this->is_token({ LPAREN }, this->position)) expression = this->parse_parenthisized();
    else if (this->is_token(unary_token_types, this->position)) expression = this->parse_unary();
    else if (this->is_token({ LSQPAREN }, this->position)) expression = this->parse_array();
    else if (this->is_token({ BEGIN }, this->position)) expression = this->parse_object();
    else if (this->is_token({ WHILE }, this->position)) expression = this->parse_while();
    else if (this->is_token({ TYPE }, this->position)) expression = this->parse_typedef();
    else if (this->is_token({ IF }, this->position)) expression = this->parse_if();

    if (expression)
    {
        AstNode* subparsed = this->subparse(expression, started_position, ignore_binaries);
        if (subparsed) expression = subparsed;
    } else this->parser_errorf("Cannot parse expression");

    return expression;
}

IfNode* Parser::parse_if()
{
    this->eat({ IF });

    AstNode* condition = this->parse_expression();
    BlockNode* success_block = this->parse_block();
    
    BlockNode* fail_block = new BlockNode({});

    if (this->is_token({ ELSE }, this->position))
    {
        this->eat({ ELSE });

        fail_block = this->parse_block();
    }

    return new IfNode(success_block, fail_block, condition);
}

ObjectNode* Parser::parse_object()
{
    eat({ BEGIN });

    vector<AstNode*> fields;

    while (!this->is_token({ END }, this->position))
    {
        AstNode* field = this->parse_expression();
        if (field)
        {
            fields.push_back(field);
            this->match({ COMMA, SEMICOLON });
        } else break;
    }
    
    this->eat({ END });

    return new ObjectNode(fields);
}

ArrayNode* Parser::parse_array()
{
    eat({ LSQPAREN });

    vector<AstNode*> elements;

    while (!this->is_token({ RSQPAREN }, this->position))
    {
        AstNode* element = this->parse_expression();
        if (element)
        {
            elements.push_back(element);
            this->match({ COMMA });
        } else break;
    }
    
    this->eat({ RSQPAREN });

    return new ArrayNode(elements);
}

AstNode* Parser::subparse(AstNode* expression, int started_position, bool ignore_binaries)
{
    AstNode* subparsed = expression;
    bool is_subparsed = false;

    if (this->is_token({ LPAREN }, this->position))
    {
        is_subparsed = true;
        subparsed = this->parse_call(expression);
    }
    else if (this->is_token({ LSQPAREN }, this->position)) 
    {
        is_subparsed = true;
        subparsed = this->parse_indexation(expression);
    }
    else if (this->is_token(binary_token_types, this->position) && !ignore_binaries)
    {
        this->position = started_position;
        is_subparsed = true;
        subparsed = this->parse_binary();
    }

    if (is_subparsed)
    {
        AstNode* deep_subparsed = this->subparse(subparsed, started_position, ignore_binaries);
        if (deep_subparsed) subparsed = deep_subparsed;
    }

    return subparsed;
}

WhileNode* Parser::parse_while()
{
    this->eat({ WHILE });

    AstNode* condition = this->parse_expression();
    BlockNode* block = this->parse_block();

    return new WhileNode(condition, block);
}

AstNode* Parser::parse_binary()
{
    AstNode* left = this->headterm();

    while (this->match({ AND, OR }))
    {
        Token* operator_token = this->tokens.at(this->position - 1);

        AstNode* right = this->headterm();

        left = new BinaryOperationNode(left, operator_token, right);
    }

    return left;
}

AstNode* Parser::headterm()
{
    AstNode* left = this->term();

    while (this->match({ ASSIGN, EQ, NOTEQ, BIGGER, SMALLER, BIGGER_OR_EQ, SMALLER_OR_EQ }))
    {
        Token* operator_token = this->tokens.at(this->position - 1);

        AstNode* right = this->term();

        left = new BinaryOperationNode(left, operator_token, right);
    }

    return left;
}

AstNode* Parser::term()
{
    AstNode* left = this->subterm();

    while (this->match({ PLUS, MINUS }))
    {
        Token* operator_token = this->tokens.at(this->position - 1);

        AstNode* right = this->subterm();

        left = new BinaryOperationNode(left, operator_token, right);
    }

    return left;
}

AstNode* Parser::subterm()
{
    AstNode* left = this->parse_expression(true);

    while (this->match({ ASTERISK, SLASH }))
    {
        Token* operator_token = this->tokens.at(this->position - 1);

        AstNode* right = this->parse_expression(true);

        left = new BinaryOperationNode(left, operator_token, right);
    }

    return left;
}

TypedefNode* Parser::parse_typedef()
{
    this->eat({ TYPE });

    IdentifierNode* id = this->parse_identifier();

    this->eat({ ASSIGN });

    AstNode* type = this->parse_expression();

    return new TypedefNode(id, type);
}

UnaryOperationNode* Parser::parse_unary()
{
    Token* token = this->eat(unary_token_types);
    AstNode* operand = this->parse_expression();

    return new UnaryOperationNode(token, operand);
}

CallNode* Parser::parse_call(AstNode* to_call)
{
    eat({ LPAREN });

    vector<AstNode*> args;

    while (!this->is_token({ RPAREN }, this->position))
    {
        AstNode* argument = this->parse_expression();
        if (argument)
        {
            args.push_back(argument);
            this->match({ COMMA });
        } else break;
    }
    
    this->eat({ RPAREN });

    return new CallNode(to_call, args);
}

IndexationNode* Parser::parse_indexation(AstNode* where)
{
    eat({ LSQPAREN });

    AstNode* index = this->parse_expression();

    eat({ RSQPAREN });

    return new IndexationNode(where, index);
}

ParenthisizedNode* Parser::parse_parenthisized()
{
    eat({ LPAREN });

    AstNode* expression = this->parse_expression();

    eat({ RPAREN });

    return new ParenthisizedNode(expression);
}

FunctionNode* Parser::parse_function()
{
    this->eat({ FUNCTION });

    IdentifierNode* identifier = this->parse_identifier();

    this->eat({ LPAREN });

    vector<IdentifierNode*> args;

    while (!this->is_token({ RPAREN }, this->position))
    {
        IdentifierNode* argument = this->parse_identifier();
        if (argument)
        {
            args.push_back(argument);
            this->match({ COMMA });
        } else break;
    }
    
    this->eat({ RPAREN });

    this->eat({ ARROW });

    AstNode* return_type = this->parse_expression();

    BlockNode* block = this->parse_block();

    return new FunctionNode(identifier, args, block, return_type);
}

BlockNode* Parser::parse_block()
{
    this->eat({ BEGIN });

    vector<AstNode*> nodes;

    while (!this->is_token({ END }, this->position))
    {
        AstNode* node = this->parse_expression();
        if (node)
        {
            nodes.push_back(node);
            this->match({ SEMICOLON });
        } else break;
    }

    this->eat({ END });

    return new BlockNode(nodes);
}

LiteralNode* Parser::parse_literal()
{
    return new LiteralNode(this->eat(literal_token_types));
}

IdentifierNode* Parser::parse_identifier()
{
    Token* start = this->eat({ IDENTIFIER });
    
    if (this->is_token({ ANNOTATE }, this->position))
    {
        this->eat({ ANNOTATE });
        AstNode* type = this->parse_expression();

        return new IdentifierNode(start, type);
    }

    return new IdentifierNode(start);
}