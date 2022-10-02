
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/semaphore.h>

#include <linux/ctype.h>
#include <linux/random.h>

/*
Function name:      readDevice
Description:        Handles device when read
*/
static ssize_t readDevice(struct file *, char __user*, size_t, loff_t *);

/*
Function name:      writeDevice
Description:        Handles device when written to
*/
static ssize_t writeDevice(struct file *, const char __user*, size_t, loff_t *);

/*
Function name:      openDevice
Description:        Handles device when opened
*/
static int openDevice(struct inode *, struct file *);

/*
Function name:      releaseDevice
Description:        Handles device when released
*/
static int releaseDevice(struct inode *, struct file *);

/*------------------------------------------------------------------------------------------*/

/*
Function name:      runCommand
Description:        parses command given
*/
void runCommand(void);

/*
Function name:      createdBoard
Description:        creates an empty board
*/
void createBoard(void);

/*
Function name:      printBoard
Description:        formats message to be read
*/
void printBoard(void);

/*
Function name:      updateBoard
Description:        updates board given directions from checkMove()
*/
void updateBoard(int, int, char);

/*
Function name:      computerRun
Description:        computer plays the game during its turn
*/
void computerRun(void);

/*
Function name:      checkMove
Description:        checks the location given and return if location is valid
*/
int checkMove(int, int, char);

/*
Function name:      checkPossibleMove
Description:        checks if the piece given has any possible move on the board
*/
int checkPossibleMove(char);

/*
Function name:      checkWin
Description:        checks if there are no more moves and messages the outcome
*/
int checkWin(void);

#define BOARD_SIZE 8

/*stores buffer*/
static char bufferStatement[67];

/*board game*/
static char boardGame[BOARD_SIZE][BOARD_SIZE];

/*pieces for board game*/
static char cpuPiece;
static char playerPiece;

static int playersTurn;
static int playingGame = 0;

/*tracks direction updates for board game*/
static int directionUp;
static int directionDown;
static int directionLeft;
static int directionRight;

static int directionTopLeft;
static int directionTopRight;
static int directionBottomLeft;
static int directionBottomRight;

struct semaphore checking;

MODULE_LICENSE("GPL");

/*file operations*/
static struct file_operations operations = {

    .owner = THIS_MODULE,
    .read = readDevice,
    .write = writeDevice,
    .open = openDevice,
    .release = releaseDevice

};

/*device*/
static struct miscdevice reversiDevice = {

    .minor = MISC_DYNAMIC_MINOR,
    .name = "reversi",
    .fops = &operations,

};

int init_module(void) {
    
    int error = misc_register(&reversiDevice);
    
    /*error with registering device*/
    if (error < 0) {

        return error;

    }

    sema_init(&checking, 1);

    /*successful register*/
    return 0;
}


void cleanup_module(void) {

    /*deregisters device*/
    misc_deregister(&reversiDevice);

}

static int openDevice(struct inode *inode, struct file *file){

    /*check if already open*/
    if (down_interruptible(&checking) != 0) {

        return -1;

    }

    try_module_get(THIS_MODULE);
    return 0;
}

static int releaseDevice(struct inode *inode, struct file *file){

    /*open up device*/
    up(&checking);
    module_put(THIS_MODULE);

    return 0;
}


static ssize_t readDevice(struct file *file, char __user *userVar, size_t length, loff_t *offset) {
    
    int i;

    /*checks length to message size*/
    if (length > sizeof(bufferStatement)) {

        length = sizeof(bufferStatement);

    }

    /*tries to copy to user*/
    if (copy_to_user(userVar, bufferStatement, length)) {

        return -EFAULT;

    }

    /*clears entire buffer*/
    for (i = 0; i < sizeof(bufferStatement); i++) {

        bufferStatement[i] = '\0';

    }

    /*success*/
    return length;
}

static ssize_t writeDevice(struct file *file, const char __user *userVar, size_t length, loff_t *offset) {

    /*checks length to message size*/
    if (length > sizeof(bufferStatement)) {

        length = sizeof(bufferStatement);

    }

    /*tries to copy from user*/
    if (copy_from_user(bufferStatement, userVar, length)) {

        return -EFAULT;
    
    }
    
    /*runs game*/
    runCommand();

    /*success*/
    return length;
}

void runCommand(void) {

    int index = 0;
    /*gets size of command*/
    while (bufferStatement[index] != '\n'){

        index++;

    }

    /*command 00, start game*/
    if (bufferStatement[0] == '0' && bufferStatement[1] == '0') {
        
        /*invalid command*/
        if (index != 4 ||
            bufferStatement[2] != ' ' ||
            (bufferStatement[3] != 'X' &&
            bufferStatement[3] != 'O')) {
            
            strcpy(bufferStatement, "INVFMT\n");
    
        }
        else {

            /*creates board*/
            createBoard();
            playingGame = 1;

            playerPiece = bufferStatement[3];
            
            /*gets CPU Piece*/
            if (playerPiece == 'X'){

                cpuPiece = 'O';

            }
            else if (playerPiece == 'O') {

                cpuPiece = 'X';

            }

            /*sets player's turn*/
            if (playerPiece == 'X') {

                playersTurn = 1;

            }
            else {

                playersTurn = 0;

            }

            strcpy(bufferStatement, "OK\n");
        }

    }
    /*command 01, print board*/
    else if (bufferStatement[0] == '0' && bufferStatement[1] == '1') {

        /*invalid command*/
        if (index != 2) {
            
            strcpy(bufferStatement, "INVFMT\n");

        } 
        /*game not started*/
        else if (playingGame == 0) {
            
            strcpy(bufferStatement, "NO GAME\n");
            
        }
        else {
           
            printBoard();

        }

    }
    /*command 02, player tries to play*/  
    else if (bufferStatement[0] == '0' && bufferStatement[1] == '2') {
        
        /*invalid command*/
        if ((index != 6) ||
            bufferStatement[2] != ' ' ||
            (isdigit(bufferStatement[3]) == 0) ||
            bufferStatement[4] != ' ' ||
            (isdigit(bufferStatement[5]) == 0)) {
            
            strcpy(bufferStatement, "INVFMT\n");

        } 
        /*game not started*/
        else if (playingGame == 0) {

            strcpy(bufferStatement, "NO GAME\n");

        }
        /*out of turn*/
        else if (playersTurn == 0) { 

            strcpy(bufferStatement, "OOT\n");

        }
        else {
        
            int getR = bufferStatement[3] - 48;
            int getC = bufferStatement[5] - 48;
            
            /*checks given location*/
            if (checkMove(getR, getC, playerPiece)) {

                updateBoard(getR, getC, playerPiece);
                playersTurn = 0;

            }
            else {

                strcpy(bufferStatement, "ILLMOVE\n");

            }

        }

    }
    /*command 03, computer plays*/
    else if (bufferStatement[0] == '0' && bufferStatement[1] == '3') {

        if (index != 2) {
            
            strcpy(bufferStatement, "INVFMT\n");

        }
        else if (playingGame == 0) {

            strcpy(bufferStatement, "NO GAME\n");

        }
        else if (playersTurn == 1) {

            strcpy(bufferStatement, "OOT\n");

        }
        else {

            computerRun();
            playersTurn = 1;

        }

    }
    /*command 04, passes player turn*/
    else if (bufferStatement[0] == '0' && bufferStatement[1] == '4') {

        /*invalid command*/
        if (index != 2) {
            
            strcpy(bufferStatement, "INVFMT\n");

        }
        /*game has not started*/
        else if (playingGame == 0){

            strcpy(bufferStatement, "NO GAME\n");

        }
        /*out of turn*/
        else if (playersTurn == 0) {

            strcpy(bufferStatement, "OOT\n");

        }
        else {
            
            /*checks if there are any possible moves*/
            if (checkPossibleMove(playerPiece)) {

                strcpy(bufferStatement, "ILLMOVE\n");

            }
            else {

                playersTurn = 0;
                strcpy(bufferStatement, "OK\n");

            }

        }

    } 
    else {

        /*command doesnt exist*/
        strcpy(bufferStatement, "UNKCMD\n");

    }

}

void createBoard(void) {

    int r;
    int c;
    
    /*creates empty board*/
    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {

            boardGame[r][c] = '-';

        }

    }

    /*starting board*/
    boardGame[3][3] = 'O';
    boardGame[3][4] = 'X';
    boardGame[4][3] = 'X';
    boardGame[4][4] = 'O';

}

void printBoard(void) {

    /*converts the board into a message*/
    int r;
    int c;

    int index = 0;

    for (r = 0; r < BOARD_SIZE; r++){
        for (c = 0; c < BOARD_SIZE; c++){

            bufferStatement[index] = boardGame[r][c];
            index++;

        }   

    }

    /*adds tab*/
    bufferStatement[index] = '\t';

    /*adds current player's piece*/
    if (playersTurn) {

        bufferStatement[index + 1] = playerPiece;

    }
    else {

        bufferStatement[index + 1] = cpuPiece;

    }

    /*adds last newline*/
    bufferStatement[index + 2] = '\n';
}

void updateBoard(int givenR, int givenC, char piece) {

    int r;
    int c;

    if (directionUp) {

        /*updates up direction*/
        for (r = givenR - 1; r >= 0; r--){
            
            if (boardGame[r][givenC] == piece) {

                r = -1;

            }
            else {

                boardGame[r][givenC] = piece;

            }
        }
    }

    if (directionDown) {

        /*updates down direction*/
        for (r = givenR + 1; r <= BOARD_SIZE - 1; r++) {

            if (boardGame[r][givenC] == piece) {

                r = BOARD_SIZE;

            }
            else {

                boardGame[r][givenC] = piece;

            }
        }
    }

    if (directionRight) {

        /*updates right direction*/
        for (c = givenC + 1; c <= BOARD_SIZE - 1; c++) {

            if (boardGame[givenR][c] == piece) {

                c = BOARD_SIZE;
            }
            else {

                boardGame[givenR][c] = piece;

            }
        }
    }

    if (directionLeft) {

        /*updates left direction*/
        for (c = givenC - 1; c >= 0; c--) {

            if (boardGame[givenR][c] == piece) {

                c = -1;
            }
            else {

                boardGame[givenR][c] = piece;

            }
        }
    }

    if (directionTopLeft){

        /*updates top right direction*/
        for (r = givenR - 1, c = givenC - 1; r >=0 && c >= 0; r--, c--) {

            if (boardGame[r][c] == piece) {

                r = -1;    
                c = -1;

            }
            else {

                boardGame[r][c] = piece;

            }
        }
    }

    if (directionTopRight){

        /*updates top left direction*/
        for (r = givenR - 1, c = givenC + 1; r >=0 && c <= BOARD_SIZE - 1; r--, c++) {

            if (boardGame[r][c] == piece) {

                r = -1;    
                c = BOARD_SIZE;

            }
            else {

                boardGame[r][c] = piece;

            }
        }
    }

    if (directionBottomLeft) {

        /*updates bottom right direction*/
        for (r = givenR + 1, c = givenC - 1; r <= BOARD_SIZE - 1 && c >= 0; r++, c--) {

            if (boardGame[r][c] == piece) {

                r = BOARD_SIZE;    
                c = -1;

            }
            else {

                boardGame[r][c] = piece;

            }
        }
    }

    if (directionBottomRight) {

        /*updates bottom left direction*/
        for (r = givenR + 1, c = givenC + 1; r <= BOARD_SIZE - 1 && c <= BOARD_SIZE - 1; r++, c++) {

            if (boardGame[r][c] == piece) {

                r = BOARD_SIZE;    
                c = BOARD_SIZE;

            }
            else {

                boardGame[r][c] = piece;

            }
        }
    }

    /*updates last piece*/
    boardGame[givenR][givenC] = piece;

    /*resets values*/
    directionUp = 0;
    directionDown = 0;
    directionLeft = 0;
    directionRight = 0;

    directionTopLeft = 0;
    directionTopRight = 0;
    directionBottomLeft = 0;
    directionBottomRight = 0;
    
    if (checkWin() == 0) {

        strcpy(bufferStatement, "OK\n");

    }
}

int checkMove(int givenR, int givenC, char piece) { 

    /*move is possible*/
    int possible = 0;

    char opponentPiece = ' ';

    /*indicates to check the piece between*/
    int checkFirstPiece = 1;

    int r;
    int c;

    /*resets values*/
    directionUp = 0;
    directionDown = 0;
    directionLeft = 0;
    directionRight = 0;

    directionTopLeft = 0;
    directionTopRight = 0;
    directionBottomLeft = 0;
    directionBottomRight = 0;


    if (piece == playerPiece){

        opponentPiece = cpuPiece;

    }
    else if (piece == cpuPiece) {

        opponentPiece = playerPiece;

    }

    if ((givenR < 0 || givenR > BOARD_SIZE - 1) || givenC < 0 || givenC > BOARD_SIZE - 1) {

        strcpy(bufferStatement, "ILLMOVE\n");

    }
    else {

        if (boardGame[givenR][givenC] != '-') {

            strcpy(bufferStatement, "ILLMOVE\n");

        } 
        else { 

            /*checks up direction*/
            for (r = givenR - 1; r >= 0; r--){
                
                /*checks first piece*/
                if (checkFirstPiece) {

                    if (boardGame[r][givenC] != opponentPiece) {
                        
                        directionUp = 0;
                        r = -1;
                    }

                    checkFirstPiece = 0;
                }
                /*possible move*/
                else if (boardGame[r][givenC] == piece) {

                    directionUp = 1;
                    possible = 1;
                    r = -1;

                }
                /*stop checking*/
                else if (boardGame[r][givenC] != opponentPiece) {

                    directionUp = 0;
                    r = -1;

                }

            }
            
            checkFirstPiece = 1;

            /*checks down direction*/
            for (r = givenR + 1; r <= BOARD_SIZE - 1; r++) {
                
                /*checks first piece*/
                if (checkFirstPiece) {

                    if (boardGame[r][givenC] != opponentPiece) {
                        
                        directionDown = 0;
                        r = BOARD_SIZE;
                    }

                    checkFirstPiece = 0;
                }
                /*possible move*/
                else if (boardGame[r][givenC] == piece) {

                    directionDown = 1;
                    possible = 1;
                    r = BOARD_SIZE;

                }
                /*stop checking*/
                else if (boardGame[r][givenC] != opponentPiece) {

                    directionDown = 0;
                    r = BOARD_SIZE;

                }


            }

            checkFirstPiece = 1;

            /*checks right direction*/
            for (c = givenC + 1; c <= BOARD_SIZE - 1; c++) {
                
                /*checks first piece*/
                if (checkFirstPiece) {

                    if (boardGame[givenR][c] != opponentPiece) {
                        
                        directionRight = 0;
                        c = BOARD_SIZE;
                    }

                    checkFirstPiece = 0;
                }
                /*possible move*/
                else if (boardGame[givenR][c] == piece) {

                    directionRight = 1;
                    possible = 1;
                    c = BOARD_SIZE;

                }
                /*stop checking*/
                else if (boardGame[givenR][c] != opponentPiece) {

                    directionRight = 0;
                    c = BOARD_SIZE;

                }

            }

            checkFirstPiece = 1;

            /*checks left direction*/
            for (c = givenC - 1; c >= 0; c--) {
                
                /*checks first piece*/
                if (checkFirstPiece) {

                    if (boardGame[givenR][c] != opponentPiece) {
                        
                        directionLeft = 0;
                        c = -1;
                    }

                    checkFirstPiece = 0;
                }
                /*possible move*/
                else if (boardGame[givenR][c] == piece) {

                    directionLeft = 1;
                    possible = 1;
                    c = -1;

                }
                /*stop checking*/
                else if (boardGame[givenR][c] != opponentPiece) {

                    directionLeft = 0;
                    c = -1;

                }

            }

            checkFirstPiece = 1;

            /*checks top left*/
            for (r = givenR - 1, c = givenC - 1; r >=0 && c >= 0; r--, c--) {
                
                /*checks first piece*/
                if (checkFirstPiece) {

                    if (boardGame[r][c] != opponentPiece) {
                        
                        directionTopLeft = 0;
                        r = -1;
                        c = -1;
                    }

                    checkFirstPiece = 0;
                }
                /*possible move*/
                else if (boardGame[r][c] == piece) {

                    directionTopLeft = 1;
                    possible = 1;
                    r = -1;
                    c = -1;

                }
                /*stop checking*/
                else if (boardGame[r][c] != opponentPiece) {

                    directionTopLeft = 0;
                    r = -1;
                    c = -1;

                }
                
            }

            checkFirstPiece = 1;

            /*checks top right*/
            for (r = givenR - 1, c = givenC + 1; r >=0 && c <= BOARD_SIZE - 1; r--, c++) {
                
                /*checks first piece*/
                if (checkFirstPiece) {

                    if (boardGame[r][c] != opponentPiece) {
                        
                        directionTopRight = 0;
                        r = -1;
                        c = BOARD_SIZE;
                    }

                    checkFirstPiece = 0;
                }
                /*possible move*/
                else if (boardGame[r][c] == piece) {

                    directionTopRight = 1;
                    possible = 1;
                    r = -1;
                    c = BOARD_SIZE;

                }
                /*stop checking*/
                else if (boardGame[r][c] != opponentPiece) {

                    directionTopRight = 0;
                    r = -1;
                    c = BOARD_SIZE;

                }
                
            }

            checkFirstPiece = 1;

            /*checks bottom left*/
            for (r = givenR + 1, c = givenC - 1; r <= BOARD_SIZE - 1 && c >= 0; r++, c--) {
                
                /*checks first piece*/
                if (checkFirstPiece) {

                    if (boardGame[r][c] != opponentPiece) {
                        
                        directionBottomLeft = 0;
                        r = BOARD_SIZE;
                        c = -1;
                    }

                    checkFirstPiece = 0;
                }
                /*possible move*/
                else if (boardGame[r][c] == piece) {

                    directionBottomLeft = 1;
                    possible = 1;
                    r = BOARD_SIZE;
                    c = -1;

                }
                /*stop checking*/
                else if (boardGame[r][c] != opponentPiece) {

                    directionBottomLeft = 0;
                    r = BOARD_SIZE;
                    c = -1;

                }
                
            }

            checkFirstPiece = 1;

            /*checks bottom right*/
            for (r = givenR + 1, c = givenC + 1; r <= BOARD_SIZE - 1 && c <= BOARD_SIZE - 1; r++, c++) {
                
                /*checks first piece*/
                if (checkFirstPiece) {

                    if (boardGame[r][c] != opponentPiece) {
                        
                        directionBottomRight = 0;
                        r = BOARD_SIZE;
                        c = BOARD_SIZE;
                    }

                    checkFirstPiece = 0;
                }
                /*possible move*/
                else if (boardGame[r][c] == piece) {

                    directionBottomRight = 1;
                    possible = 1;
                    r = BOARD_SIZE;
                    c = BOARD_SIZE;

                }
                /*stop checking*/
                else if (boardGame[r][c] != opponentPiece) {

                    directionBottomRight = 0;
                    r = BOARD_SIZE;
                    c = BOARD_SIZE;

                }
                
            }
        }

    }

    return possible;
}

int checkPossibleMove(char piece){
    
    int r;
    int c;

    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {

            /*if move found*/
            if (checkMove(r, c, piece)) {
                return 1;
            }
        }
    }

    /*no possible moves*/
    return 0;
}

void computerRun(void) {

    /*CPU can play move*/
    int possibleCPU = checkPossibleMove(cpuPiece);

    /*random number*/
    unsigned int randomR;
    unsigned int randomC;

    get_random_bytes(&randomR, sizeof(randomR));
    randomR %= BOARD_SIZE;

   
    get_random_bytes(&randomC, sizeof(randomC));
    randomC %= BOARD_SIZE;

    /*possible play for CPU*/
    if (possibleCPU == 1){
        
        /*keeps checking moves*/
        while (checkMove(randomR, randomC, cpuPiece) == 0){

            get_random_bytes(&randomR, sizeof(randomR));
            randomR %= BOARD_SIZE;

            get_random_bytes(&randomC, sizeof(randomC));
            randomC %= BOARD_SIZE;

        }

        updateBoard(randomR, randomC, cpuPiece);
    }

}

int checkWin(void) {
    
    int checkCPU = checkPossibleMove(playerPiece);
    int checkPlayer = checkPossibleMove(cpuPiece);

    /*no possible moves*/
    if ((checkCPU == 0) && (checkPlayer == 0)) {
        
        int countCPU = 0;
        int countPlayer = 0;

        int r;
        int c;

        /*counts number of pieces*/
        for (r = 0; r < BOARD_SIZE; r++) {
            for (c = 0; c < BOARD_SIZE; c++) {

                if (boardGame[r][c] == playerPiece) {

                    countPlayer++;

                }
                else if (boardGame[r][c] == cpuPiece) {

                    countCPU++;

                }
            }
        }

        /*player wins*/
        if (countPlayer > countCPU) {

            strcpy(bufferStatement, "WIN\n");
            
        }
        /*player loses*/
        else if (countPlayer < countCPU) {

            strcpy(bufferStatement, "LOSE\n");

        }
        /*player ties*/
        else {

            strcpy(bufferStatement, "TIE\n");

        }

        playingGame = 0;
        return 1;
    }

    return 0;
}