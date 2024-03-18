# Bash Shell In C
## A bash implementation created using C

Hi, the following code is my implementation for Bash (Shell) in C, and it contains some of the functionallities that the shell has. 

## Commands
*history* - details the commands history of the user (ascending)
*cd* (almost fully developed) - the regular change directory command (supports blank cd, cd .. and cd [DIR_PATH])
*ls* (not fully functioning) - the regular ls command, does support file permission details (rwx+)
*pwd* - straight forward directory prompt
*mkdir* (fully functioning) - creates a directory (supports blank args and given path)
*rm* - removing a file / directory from the system (supportes -r)
*file* - printing details about an attached file:
  -> Owner
  -> Size
  -> Group
  -> Permissions (under development)

*cat* - printing the contents of a file
*echo* (not fully functioning) - basic echo functionallity.

## File Execution

The shell could run a binary (.bin) files using './' or '.' before the name of the file. (supports paths)

## Piping 

Under development, but still working for some commands: echo, cat, ls

## Subproccessing

The shell is completely asynchronous


