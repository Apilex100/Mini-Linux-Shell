# Mini-Shell

## Brief Description
A simple implementation of the Unix Shell in the C Programming language. This project was coded and tested in Ubuntu 20.04 .

## How to run
1. Compile and run `miniShell.c`
2. The MY_SHELL (Mini Shell) is now yours to try!

## Detailed Description
### Implementation Details
MY_SHELL has both built-in and external commands. The built-in commands are implemented in `miniShell.c`. The external commands are implemented in `.c` files with the name of the command in the cmds directory (for eg: `ls` is implemented in `cmds/ls.c` ). 

When you type a command into MY_SHELL, it first looks for the command in the list of built-ins that it maintains. 
 * If present, it will call the corresponding function (the mapping from built-in command name to the command function is implemented using **function pointers** for better performance and to eliminate the need for cumbersome switch case statements). 
 * If not, it will fork a new process, load the command's image into the child process and wait for the child process to finish execution before displaying the prompt again.
 * If the command is not found (the corresponding `.c` file is not found), an error message indicating that the command was not found will be displayed.
  
  When errors occur, appropriate error messages will be displayed.
  
  ### Commands
  Only a very small number of commands have been implemented. The commands are:
  * `cd`
  * `help`
  * `exit`
  * `pwd`
  * `echo`
  * `clear`
  * `ls`
  * `mkdir`
  * `rmdir`
  * `cat`

