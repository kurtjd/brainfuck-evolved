/*************************************************************************************************************
 * Basic Brainfuck Interpreter                                                                               *
 * Written by Kurtis Dinelle (http://kurtisdinelle.com)                                                      *
 *                                                                                                           *
 * A simple brainfuck interpreter written to be used only with the Brainfuck Evolved program.                *
 * Therefore, this interpreter does not include features a complete interpreter would have (such as input).  *
 *                                                                                                           *
 * Since brainfuck isn't fully defined, some assumptions had to be made:                                     *
 *                                                                                                           *
 *      -The size of the 'tape' holding cells will be fixed-length (1000 cells, chosesn arbitrarily).        *
 *      -Each cell can hold an unsigned char (so up to a value of 255). Going over will cause wrap-around.   *
 *      -Going outside the bounds of a type's size is actually undefined behavior in C++,                    *
 *       however most implementations handle this via wrap-around. This interpreter relies on such behavior. *
 *************************************************************************************************************/

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>

class Interpreter
{
private:
    // The max number of cycles the program can run before being assumed it is stuck in an infinite loop.
    static const unsigned MAX_CYCLES = 1000;
    static const unsigned TAPE_SIZE = 1000;  // The size of the tape. Subject to change.

    unsigned char tape[TAPE_SIZE];  // The memory tape.

    unsigned instruction_pntr;  // The index of the current instruction being executed
    int tape_pntr;  // The index of the current cell in the tape that the interpreter is pointing to.

    std::string program;  // The current program being executed.
    std::string output;  // The programs complete output to be returned at the end of execution.
    bool has_error;  // True if the interpreter ever encounters an error in the program.
    unsigned total_cycles;  // The number of cycles the program has been running for.

    void inc_pntr();  // Increments tape_pntr
    void dec_pntr();  // Decrements tape_pntr
    void inc_byte();  // Increments the value of the byte stored in the tape at tape_pntr
    void dec_byte();  // Decrements the value of the byte stored in the tape at tape_pntr
    void out_byte();  // Adds the ascii value of the byte pointed to to the programs output
    void out_byte_as_int();  // Adds the integer value of the byte pointed to to the programs output. Only used for debugging.

    void begin_loop();  // Decides whether to loop or jump out of loop
    void end_loop(); // Same as above, except kind of inverted.

    int find_loop_match(int direction);  // Finds the matching loop bracket and returns its index.
    bool check_syntax();  // Checks to make sure all ['s have matching ]'s.
    void reset();  // Resets all variables.

public:
    Interpreter();

    // Loops through the entire program, character-by-character, and interprets it, then returns any output.
    std::string run(const std::string &prgrm);

    static const std::string ERROR;  // The output returned for erroneous programs.

};

#endif
