#include <string>
#include <iostream>
#include <sstream>
#include "interpreter.h"

const std::string Interpreter::ERROR = "Error";

Interpreter::Interpreter()
{
    this->reset();
}

std::string Interpreter::run(const std::string &prgrm)
{
    this->reset();
    this->program = prgrm;

    if(!this->check_syntax())
        return Interpreter::ERROR;

    while(this->instruction_pntr < this->program.length())
    {
        // If the program throws an error or is running too long, terminate it.
        if(this->total_cycles > this->MAX_CYCLES || this->has_error)
            return Interpreter::ERROR;

        // Now we decide which operation to perform based on the character.
        switch(this->program[this->instruction_pntr])
        {
        case '>':
            this->inc_pntr();
            break;
        case '<':
            this->dec_pntr();
            break;
        case '+':
            this->inc_byte();
            break;
        case '-':
            this->dec_byte();
            break;
        case '.':
            this->out_byte();
            break;
        case '#':  // This was only used for debugging purposes, and is not an actual command.
            this->out_byte_as_int();
            break;
        case '[':
            this->begin_loop();
            break;
        case ']':
            this->end_loop();
            break;
        default:
            break;
        }

        ++this->instruction_pntr;
        ++this->total_cycles;
    }

    return this->output;
}

void Interpreter::inc_pntr()
{
    ++this->tape_pntr;

    if(this->tape_pntr >= static_cast<int>(this->TAPE_SIZE))
        this->has_error = true;
}

void Interpreter::dec_pntr()
{
    --this->tape_pntr;

    if(this->tape_pntr < 0)
        this->has_error = true;
}

void Interpreter::inc_byte()
{
    ++this->tape[this->tape_pntr];
}

void Interpreter::dec_byte()
{
    --this->tape[this->tape_pntr];
}

void Interpreter::out_byte()
{
    this->output += this->tape[this->tape_pntr];
}

void Interpreter::out_byte_as_int()
{
    std::stringstream int_val;
    int_val << static_cast<int>(this->tape[this->tape_pntr]);  // Need to cast char to int or else the ASCII value will be used.
    this->output += int_val.str();  // Then add the string representation of the number to the output.
}


// If the byte currently being pointed to is zero, jump to the end of the loop. If not, continue with normal execution flow.
void Interpreter::begin_loop()
{
    if(!this->tape[this->tape_pntr])
        this->instruction_pntr = find_loop_match(1) - 1;
}

// If the byte currently being pointed to is nonzero, jump to the beginning of the loop. If not, continue with normal execution flow.
void Interpreter::end_loop()
{
    if(this->tape[this->tape_pntr])
        this->instruction_pntr = find_loop_match(-1);
}

// Find the index of a matching loop bracket.
int Interpreter::find_loop_match(int direction)
{
    int num_nested_loops = 1;  // We count the current loop as a nested loop.
    int tmp_instr_pntr = this->instruction_pntr + direction;

    /* We need to walk through the program to figure out where the end of the loop is, by counting how many nested loops there are.
       Count either backwards or forwards depending on direction. */
    while(num_nested_loops > 0)
    {
        if(tmp_instr_pntr < 0 || tmp_instr_pntr >= static_cast<int>(this->program.length()))
        {
            this->has_error = true;
            return -1;  // -1 has no meaning. Just needed to return.
        }

        char c = this->program[tmp_instr_pntr];

        if(c == '[')
            num_nested_loops += (direction > 0 ? 1 : -1);
        else if(c == ']')
            num_nested_loops += (direction > 0 ? -1 : 1);

        tmp_instr_pntr += direction;
    }

    return tmp_instr_pntr;
}

// Runs through the program making sure all loop brackets match up.
bool Interpreter::check_syntax()
{
    for(size_t i = 0; i < this->program.length(); ++i)
    {
        // The find_loop_match() method relies on the instruction_pntr variable
        this->instruction_pntr = i;

        if(this->program[i] == '[')
            this->find_loop_match(1);
        else if(this->program[i] == ']')
            this->find_loop_match(-1);

        if(this->has_error)
            return false;
    }

    // Needs to be reset for when the program actually begins execution.
    this->instruction_pntr = 0;

    return true;
}

void Interpreter::reset()
{
    this->instruction_pntr = 0;
    this->tape_pntr = 0;
    this->program = "";
    this->output = "";
    this->has_error = false;
    this->total_cycles = 0;

    // Initialize the tape.
    for(unsigned i = 0; i < this->TAPE_SIZE; ++i)
        this->tape[i] = 0;
}
