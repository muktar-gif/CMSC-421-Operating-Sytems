# CMSC-421-Operating-Sytems
# Unix Shell Program
The program is a simple version of a unix shell. The program supports the execution of different shell commands along with support for custom commands. Basic commnads are executed by creating a new child process and independently ran. The custom command "proc" has its own configuration and it allows the user to display information from Linux's /proc filesystem.
# Linux Kernel IPC Communication
It is a new version of a Linux kernal with added system calls that allows additional interprocess communction functionality. It contains custom system calls such as create, remove, send, and etc. that allows for more control over IPC messages.
# Reversi Virtual Charac Dev
The program a loadable kernal module which adds a virtual device driver. The main component of this project is the virtual character device that allows the user to play a game of Reversi/Othello against a CPU or another player. It is played through a series of file read and write operations to the module's /dev directory which contains the character device file.
