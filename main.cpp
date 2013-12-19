/**************************************************************************************************************
 * Brainfuck Evolved                                                                                          *
 * Written by Kurtis Dinelle (http://kurtisdinelle.com)                                                       *
 *                                                                                                            *
 * This program attemps to write programs of its own in the brainfuck language, using a genetic algorithm.    *
 * The fitness function will take into account how close the output matches, and how concise the program is.  *
 * Shorter programs receive a slight bonus.                                                                   *
 **************************************************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "interpreter.h"

// Don't modify this group of constants.
const unsigned CHAR_SIZE = 255;  // The max value of a 'cell' in the memory tape.
const unsigned NUM_MUTATIONS = 3;  // The number of types of mutations: (ADD, DELETE, CHANGE).
const char INSTRUCTIONS[] = {'+', '-', '>', '<', '[', ']', '.'};  // The list of brainfuck instructions.
const unsigned NUM_INSTRUCTIONS = (sizeof(INSTRUCTIONS) / sizeof(INSTRUCTIONS[0]));  // Holds the number of instructions allowed.
const unsigned NUM_CHILDREN = 2;  // Number of children two parents create upon reproduction.

// Modify any constants below.
const unsigned POP_SIZE = 10;  // The size of the population. This always remains the same between generations.
const unsigned MIN_PROGRAM_SIZE = 10;  // The minimum size a possible program can be.
const unsigned MAX_PROGRAM_SIZE = 500;  // The maximum size a possible program can be.
const double MUTATION_RATE = 0.01;  // The chance of a 'gene' in a child being mutated.
const double ERROR_SCORE = 1.0;  // The score an erroneous program receives.
const double LENGTH_PENALTY = 0.001;  // The size of the program is multiplied by this then added to score.
const unsigned DISPLAY_RATE = 10000;  // How often to display the best program so far.

// These aren't constant because they can be changed by the user.
std::string GOAL_OUTPUT = "Computerphile";
size_t GOAL_OUTPUT_SIZE = GOAL_OUTPUT.length();



/* These two functions used to generate either a random double or unsigned int.
   Why didn't I just overload one function? Slipped my mind at the time. */
double get_random(double low, double high)
{
    return low + static_cast<double>(rand()) / static_cast<double>(static_cast<double>(RAND_MAX) / (high - low));
}

unsigned get_random_int(unsigned low, unsigned high)
{
    return (rand() % (high - low + 1)) + low;
}


char get_random_instruction()
{
    return INSTRUCTIONS[get_random_int(0, NUM_INSTRUCTIONS - 1)];
}


void add_instruction(std::string &program, unsigned index)
{
    std::string instr(1, get_random_instruction());  // Need to convert char to string for use by insert.

    if((program.length() + 1) <= MAX_PROGRAM_SIZE)
        program.insert(index, instr);
}


void remove_instruction(std::string &program, unsigned index)
{
    // Subtract 2 instead of 1 to account for the off-by-one difference between a string length and the last element index.
    if((program.length() - 2) >= MIN_PROGRAM_SIZE)
        program.erase(index, 1);
}


void mutate_instruction(std::string &program, unsigned index)
{
    program[index] = get_random_instruction();
}


// Creates a random program by first randomly determining its size and then adding that many instructions randomly.
std::string create_random_program()
{
    std::string program;
    unsigned program_size = get_random_int(MIN_PROGRAM_SIZE, MAX_PROGRAM_SIZE);

    for(unsigned i = 1; i <= program_size; ++i)
        program += get_random_instruction();

    return program;
}


// Creates the first generation's population by randomly creating programs.
void initialize_population(std::string programs[])
{
    for(unsigned i = 0; i < POP_SIZE; ++i)
        programs[i] = create_random_program();
}


// The Fitness Function. Determines how 'fit' a program is using a few different criteria.
double calculate_fitness(const std::string &program, Interpreter &bf)
{
    // The score of the worst program possible (Besides erroneous program, and not taking into account program length).
    double max_score = GOAL_OUTPUT_SIZE * CHAR_SIZE;
    double score = 0;
    double final_score;

    std::string output = bf.run(program);

    // Impose a very large penalty for error programs, but still allow them a chance at reproduction for genetic variation.
    if(output == Interpreter::ERROR)
        return ERROR_SCORE;

    /* We need to know whether the goal output or the program's output is larger
       because that's how many iterations of the next loop need to be done. */
    std::string min_str = (output.length() < GOAL_OUTPUT.length()) ? output : GOAL_OUTPUT;
    std::string max_str = (output.length() >= GOAL_OUTPUT.length()) ? output : GOAL_OUTPUT;

    // The more each character of output is similar to its corresponding character in the goal output, the higher the score.
    for(size_t i = 0; i < max_str.length(); ++i)
    {
        unsigned output_char = (i < min_str.length()) ? min_str[i] : max_str[i] + CHAR_SIZE;
        score += abs(output_char - max_str[i]);
    }

    score += (program.length() * LENGTH_PENALTY);  // Impose a slight penalty for longer programs.

    /* The lower the score of a program, the better (think golf).
       However other calculations in the program assume a higher score is better.
       Thus, we subtract the score from max_score to get a final score. */
    final_score = max_score - score;

    return final_score;
}


/* Generates a fitness score for each program in the population.
   This is done by running the program through the brainfuck interpreter and scoring its output.
   Also returns the best program produced. */
std::string score_population(const std::string programs[], double scores[], int &worst_index, Interpreter &bf)
{
    std::string best_program;
    double best_score = 0;
    double worst_score = 9999;  // Arbitrarily high number.

    for(unsigned i = 0; i < POP_SIZE; ++i)
    {
        scores[i] = calculate_fitness(programs[i], bf);

        if(scores[i] > best_score)
        {
            best_program = programs[i];
            best_score = scores[i];
        }
        else if(scores[i] < worst_score)
        {
            worst_index = i;
            worst_score = scores[i];
        }
    }

    return best_program;
}


// Adds every program's fitness score together.
double pop_score_total(const double scores[])
{
    double total = 0;

    for(unsigned i = 0; i < POP_SIZE; ++i)
        total += scores[i];

    return total;
}


/* Selects a parent to mate using fitness proportionate selection.
   Basically, the more fit a program is, the more likely it is to be selected. */
std::string select_parent(const std::string programs[], const double scores[], const std::string &other_parent = "")
{
    double program_chances[POP_SIZE];  // Holds each program's chance of being selected (a # between 0 and 1).
    double score_total = pop_score_total(scores);
    double rand_val = get_random(0.0, 1.0);

    for(unsigned i = 0; i < POP_SIZE; ++i)
    {
        // Cast i to int so when we go to subtract 1, if i is 0 it doesn't overflow (as an unsigned int can't be negative).
        double prev_chance = ((static_cast<int>(i) - 1) < 0) ? 0 : program_chances[i - 1];

        /* We add the previous program's chance to this program's chance because that is its range of being selected.
           The higher the fitness score, the bigger this program's range is. */
        program_chances[i] = (scores[i] / score_total) + (prev_chance);

        // Need to subtract a small amount from rand_val due to floating-point precision errors. Without it, equality check could fail.
        if(program_chances[i] >= (rand_val - 0.001) && (programs[i] != other_parent))
            return programs[i];
    }

    /* If the other parent was the last program in the list, we might get here.
       In that case, just return the 1st program. */
    return programs[0];
}


/* Mutates a program by either inserting, removing, or changing an instruction.
   This returns a string rather than modifying a a string just due to the way it is used in the program. */
std::string mutate(std::string child)
{
    // Loop through each command and randomly decide to mutate it based on the mutation rate.
    for(size_t i = 0; i < child.length(); ++i)
    {
        if(MUTATION_RATE >= get_random(0.0, 1.0))
        {
            unsigned mutation_type = get_random_int(1, NUM_MUTATIONS);

            switch(mutation_type)
            {
            case 1:
                mutate_instruction(child, i);
                break;
            case 2:
                add_instruction(child, i);
                break;
            case 3:
                remove_instruction(child, i);
                break;
            default:
                break;
            }
        }
    }

    return child;
}


// Performs crossover between two parents to produce two children.
void mate(const std::string &parent1, const std::string &parent2, std::string children[])
{
    // We need to find which program is longest.
    std::string min_str = (parent1.length() < parent2.length()) ? parent1 : parent2;
    std::string max_str = (parent1.length() >= parent2.length()) ? parent1 : parent2;

    // Determine a crossover point at random.
    unsigned crosspoint = get_random_int(1, max_str.length() - 1);

    // Find the substring of the program after the crossover point
    std::string max_str_contrib = max_str.substr(crosspoint);

    // Then erase after that point
    max_str.erase(crosspoint);

    /* If the cross-over point is less than the length of the smaller program, then we need to combine part of it with the larger program.
       If not, then we do nothing and just take part of the larger program and add it to it. */
    if(crosspoint <= min_str.length())
    {
        max_str += min_str.substr(crosspoint);
        min_str.erase(crosspoint);
    }

    // Add the 2nd part of the larger program to the smaller program.
    min_str += max_str_contrib;

    // Call the mutate function on the children which has a small chance of actually causing a mutation.
    children[0] = mutate(min_str);
    children[1] = mutate(max_str);
}


bool program_exists(const std::string &program, const std::string programs[])
{
    for(unsigned i = 0; i < POP_SIZE; ++i)
    {
        if(program == programs[i])
            return true;
    }

    return false;
}


void replace_program(const std::string &parent, const std::string &child, std::string programs[])
{
    for(unsigned i = 0; i < POP_SIZE; ++i)
    {
        if(parent == programs[i])
        {
            programs[i] = child;
            break;
        }
    }
}


int main(int argc, char *argv[])
{
    // Check if ran from command line.
    if(argc > 1)
    {
        GOAL_OUTPUT = argv[1];
        GOAL_OUTPUT_SIZE = GOAL_OUTPUT.length();
    }

    // Initialize the brainfuck interpreter and seed random.
    Interpreter brainfuck;
    srand(time(0));

    std::string programs[POP_SIZE];
    double fitness_scores[POP_SIZE];

    initialize_population(programs);

    std::string best_program;

    bool keep_going = false;  // Just used to have the program keep searching after a match is found.

    unsigned long generations = 0;

    // And now we just repeat the process of selection and reproduction over and over again.
    while(1)
    {
        int worst_program_index = 0;

        best_program = score_population(programs, fitness_scores, worst_program_index, brainfuck);

        // Select two parents randomly using fitness proportionate selection
        std::string parent1 = select_parent(programs, fitness_scores);
        std::string parent2 = select_parent(programs, fitness_scores, parent1);

        // Mate them to create children
        std::string children[NUM_CHILDREN];
        mate(parent1, parent2, children);

        /* Replace the parent programs with the children. We replace the parents to lessen the chance of premature convergence.
           This works because by replacing the parents, which are most similar to the children, genetic diversity is maintained.
           If the parents were not replaced, the population would quickly fill with similar genetic information. */
        replace_program(parent1, children[0], programs);
        replace_program(parent2, children[1], programs);

        /* Replace the worst program with the best program if it was replaced by its child. (Elitism).
           This is done to ensure the best program is never lost. */
        if(!program_exists(best_program, programs))
            programs[worst_program_index] = best_program;

        // Report on the current best program every so often.
        if(!(generations % DISPLAY_RATE))
        {
            std::cout << "\n\nBest program evolved so far: " << std::endl;
            std::cout << best_program << std::endl;

            std::string output = brainfuck.run(best_program);
            std::cout << "\nOutput: " << output << std::endl;

            if(output == GOAL_OUTPUT && !keep_going)
            {
                std::cout << "\n\a\a\aProgram evolved!" << std::endl;
                std::cout << "Save source code as a text file? (y/n) ";

                char answer;
                std::cin >> answer;

                if(answer == 'y')
                {
                    std::ofstream srcfile("bfsrc.txt");
                    srcfile << GOAL_OUTPUT << ":\n\n" << best_program;
                    std::cout << "Source code saved as 'bfsrc.txt'\n" << std::endl;
                }

                //std::cout << "It took roughly " << generations << " generations to evolve this program." << std::endl;
                std::cout << "Keep evolving for more efficiency? (y/n) ";
                std::cin >> answer;

                // Quit the program if the user doesn't want to continue.
                if(answer != 'y')
                    return 0;

                keep_going = true;
            }
        }

        ++generations;
    }

    return 0;
}
