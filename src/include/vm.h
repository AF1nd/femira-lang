#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <stack>
#include <map>

using namespace std;

enum Opcode 
{
    ADD = 0x01,
    SUB = 0x02,
    MUL = 0x03,
    DIV = 0x04,

    PUSHV = 0x05,

    PRINT = 0x06,
    WAIT = 0x07,

    RETURN = 0x08,
};

struct Object
{
    virtual string tostring() { return "unknown datatype"; };
};

struct String : Object 
{
    string data;
    String(string data) { this->data = data; };

    string tostring() override 
    {
        return this->data;
    }
};

struct Integer : Object
{
    int data;
    Integer(int data) { this->data = data; };
    
    string tostring() override 
    {
        return to_string(this->data);
    }
};

struct Double : Object
{
    double data;
    Double(double data) { this->data = data; };

    string tostring() override 
    {
        return to_string(this->data);
    }
}; 

struct Null : Object {
    string tostring() override 
    {
        return "null";
    }
};

struct Instruction
{
    Opcode opcode;
    Object* data;
    Instruction(Opcode opcode, Object* data = nullptr) { this->opcode = opcode; this->data = data; };
};

using Bytecode = vector<Instruction>;

class VirtualMachine 
{
    private:
        Bytecode running_bytecode;
        stack<Object*> run_stack;
    public:
        void runf_bytecode(const Bytecode bytecode, const bool trace = false);
        void errorf(const string text);

        void push_stack(Object* data);
        Object* pop_stack();
};