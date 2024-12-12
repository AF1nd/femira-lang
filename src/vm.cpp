#include <string>
#include <iostream>
#include <vector>
#include <stack>
#include <chrono>
#include <thread>

#include "include/vm.h"

using namespace std;

map<Opcode, string> opcode_to_string = {
    { ADD, "add" },
    { SUB, "sub" },
    { MUL, "mul" },
    { DIV, "div" },

    { PUSHV, "pushv" },

    { PRINT, "print" },
    { WAIT, "wait" },

    { RETURN, "return" },
};

void VirtualMachine::runf_bytecode(const Bytecode bytecode, const bool trace) 
{
    this->running_bytecode = bytecode;

    if (trace)
    {
        for (Instruction instruction: bytecode)
        {
            Opcode opcode = instruction.opcode;
            Object* data = instruction.data;
            cout << opcode_to_string[opcode] << "    " << (data != nullptr ? data->tostring() : "") << endl;
        }
    }

    for (Instruction instruction: bytecode)
    {
        Opcode opcode = instruction.opcode;
        Object* data = instruction.data;

        switch (opcode)
        {
        case ADD:
            {
                Object* obj1 = pop_stack();
                Object* obj2 = pop_stack();

                if (Integer* integer = dynamic_cast<Integer*>(obj1)) 
                {
                    if (Integer* integer2 = dynamic_cast<Integer*>(obj2)) 
                    {
                        push_stack(new Integer(integer->data + integer2-> data));
                    }
                } else if (Double* double_value = dynamic_cast<Double*>(obj1)) 
                {
                    if (Double* double_value2 = dynamic_cast<Double*>(obj2)) 
                    {
                        push_stack(new Double(double_value->data + double_value2-> data));
                    }
                }
            }
            break;
        case PUSHV:
            {
                push_stack(data);
            }
            break;
        case RETURN:
            {
                return;
            }
            break;
        case PRINT:
            {
                string to_print = pop_stack()->tostring();

                cout << " ";

                for (size_t i = 0; i < to_print.length() + 4; i++)
                {
                    cout << "-";
                }

                cout << endl;

                cout << " | " + to_print + " | " << endl;

                cout << " ";
                
                for (size_t i = 0; i < to_print.length() + 4; i++)
                {
                    cout << "-";
                }

                cout << endl;
            }
            break;
        case WAIT:
            {
                Object* object = pop_stack();

                if (Integer* integer = dynamic_cast<Integer*>(object)) 
                {
                    this_thread::sleep_for(chrono::duration<int>(integer->data));
                } else if (Double* double_value = dynamic_cast<Double*>(object)) 
                {
                    this_thread::sleep_for(chrono::duration<double>(double_value->data));
                }
            }
            break;
        default:
            break;
        }
    }
}

void VirtualMachine::errorf(const string text) 
{
    throw runtime_error("Runtime error: " + text);
}

void VirtualMachine::push_stack(Object* data)
{
    if (data == nullptr) this->errorf("Cannot push null pointer to stack");
    this->run_stack.push(data);
}

Object* VirtualMachine::pop_stack() 
{
    Object* top = this->run_stack.top();
    if (top == nullptr) this->errorf("Stack is empty");

    this->run_stack.pop();

    return top;
}