Brainfuck Evolved
=================
A little while back I had learned about genetic algorithms, and became very excited with the concept. After researching them a bit and playing around with them, I figured it'd be cool to see if I could apply genetic programming techniques myself. I didn't expect much initially, but I was impressed with the final result.

The program uses a genetic algorithm to select and breed brainfuck programs that are scored by how closely their output matches the desired output, using an internal brainfuck interpreter I wrote. Beginning with an initial randomly generated population, this process is repeated over and over again, until finally a brainfuck program is created that can output strings such as "Hello, world!" or whatever else the user wants.

Build Procedures
================
```g++ interpreter.cpp interpreter.h main.cpp -o bfevolved```

Run
===
```./bfevolved```
