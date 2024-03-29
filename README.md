# Cat and Mouse Game

The following image is a screenshot of the game simulated on CPUlator:

<img src="https://github.com/casseyshao/Cat_Mouse_Game/blob/main/game_screenshot.png" data-canonical-src="https://github.com/casseyshao/Cat_Mouse_Game/blob/main/game_screenshot.png" width="450" height="590"/>

This project was completed by Valentina and I.
It is played on the DE1-SoC board.

It can be simulated on CPUator by copying the array definitions in visuals.h to their respective functions in the cat_and_mouse_game_code.c file (equivalent to cat_mouse_game_testing.c).
The warnings on CPUlator that need to be disabled are:
1. Device-specific warnings
2. Function nesting too deep
3. Interrupt nesting too deep

Next-Step: clean code by making one function for drawing screen images, and passing pixel array to be plotted as an argument. (This was not done previously for readability, as the simulator we used to test the game required all code to be in one file to compile).

References:  
[1] ECE243 Winter 2020 - Lab 7  
[2]	W. D. Pullen, Think Labyrinth: Maze Algorithms. [Online]. Available: http://www.astrolog.org/labyrnth/algrithm.htm. [Accessed: 09-Apr-2020].  
[3]	TheJollySin, “What's a good algorithm to generate a maze?,” Stack Overflow. [Online]. Available:https://stackoverflow.com/questions/38502/whats-a-good-algorithm-to-generate-a-maze. [Accessed: 09-Apr-2020].  
[4]	A Lone Coder, “Programming Mazes”, YouTube.  [Online]. Available:https://www.youtube.com/watch?v=Y37-gB83HKE [Accessed: 09-Apr-2020].  
[5]	G. Cope, Maze Generation Algorithm - Depth First Search. [Online]. Available: https://www.algosome.com/articles/maze-generation-depth-first.html.[Accessed: 09-Apr-2020].

