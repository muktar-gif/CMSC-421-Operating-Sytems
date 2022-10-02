#include "mailboxSyscalls.h"


int main(int argc, char* argv[]){

    //calling functions without creating any mboxes

    printf("Number of mailboxes: %ld", count_mbox_421_syscall());
    perror("Call count mb");
    errno = 0;

    printf("\n");

    long getReset = reset_mbox_421_syscall();
    printf("Deleted %d mailboxes, if root user\n", getReset);
    perror("Check reset");
    errno = 0;

    printf("\n");

    //bad syscal calls
    send_msg_421_syscall(0, NULL, 0);
    perror("Bad send call");
    errno = 0;

    recv_msg_421_syscall(0, NULL, 0);
    perror("Bad receive call");
    errno = 0;

    peek_msg_421_syscall(0, NULL, 0);
    perror("Bad peek call");
    errno = 0;

    printf("\n");

    printf("Message Length, with no messages or mailboxes: %ld\n",len_msg_421_syscall(0));
    perror("Bad message call");
    errno = 0;

    printf("\n");
    
    long createMessage;

    printf("Creating mailboxes:\n");

    for (int i = 0; i < 4; i++) {

        createMessage = create_mbox_421_syscall(i);
        perror("Check create");
        errno = 0;

    }

    //check duplicate mailbox
    printf("Creating duplicate mailboxes:\n");

    create_mbox_421_syscall(0);
    perror("Check create (should fail)");
    errno = 0;

    printf("\n");

    createMessage = create_mbox_421_syscall(203);
    perror("Create MB ID 203");
    errno = 0;
    
    printf("\n");

    //checking sending messages
    printf("Sending messages: \n");

    //id 0
    send_msg_421_syscall(0, "Hello 0", 7);
    perror("Send Message to ID 0");
    errno = 0;


    send_msg_421_syscall(0, "Goodbye 0", 9);
    perror("Send Message to ID 0");
    errno = 0;

    //id 1
    send_msg_421_syscall(1, "Turn Around", 4);
    perror("Send Message to ID 1");
    errno = 0;

    //id 2
    send_msg_421_syscall(2, "welcome", 5);
    perror("Send Message to ID 2");
    errno = 0;

    send_msg_421_syscall(2, "", 7);
    perror("Send Message to ID 2");
    errno = 0;

    send_msg_421_syscall(2, "I forgot my last message", 16);
    perror("Send Message to ID 2");
    errno = 0;

    //ID 3, gets no messages

    printf("\n");

    //Testintg print
    print_mbox_421_syscall(0);
    perror("Print ID 0 to prink");
    errno = 0;

    printf("\n");

    //check list
    long *getMB = (long*) malloc(sizeof(long) * 5);

    list_mbox_421_syscall(getMB, 5);
    perror("Check list call");
    errno = 0;

    for (int i  = 0; i < 5; i++)
        printf("ID LIST: %d\n", getMB[i]);

    free(getMB);

    printf("\n");

    //Checking messages in 2
    printf("Number of messages in ID 2: %ld\n", count_msg_421_syscall(2));
    perror("Call count");
    errno = 0;

    printf("Next message len ID 2: %ld\n", len_msg_421_syscall(2));
    perror("Call len");
    errno = 0;

    char *getMsg = (char*) malloc(5);

    //recv messages for 2
    recv_msg_421_syscall(2, getMsg, 5);

    //print message
    printf("Message in ID 2: %s\n", getMsg);

    free(getMsg);

    //Test affect of recv on mailbox
    printf("Number of messages in ID 2: %ld\n", count_msg_421_syscall(2));
    perror("Call count");
    errno = 0;
    
    printf("\n");

    //Checking messages in 2
    printf("Number of messages in ID 0: %ld\n", count_msg_421_syscall(0));
    perror("Call count");
    errno = 0;

    char *getPeek = (char*) malloc(5);

    //peeking message
    peek_msg_421_syscall(2, getPeek, 5);

    printf("Message peek in ID 0: %s\n", getPeek);

    free(getPeek);

    //Test affect of peek on mailbox
    printf("Number of messages in ID 0: %ld\n", count_msg_421_syscall(0));
    perror("Call count");
    errno = 0;

    printf("\n");

    //Testing mailboxes with no messages
    printf("Number of messages in ID 3: %ld\n", count_msg_421_syscall(3));
    perror("Call count");
    errno = 0;

    printf("Next message len, message empty: %ld\n", len_msg_421_syscall(3));
    perror("Call len");
    errno = 0;

    printf("\n");

    //Testing remove with mailbox with messages
    remove_mbox_421_syscall(1);
    perror("Remove MB with messages, should fail");
    errno = 0;

    printf("\n");

    //Testing remove ID
    remove_mbox_421_syscall(3);
    perror("Remove MB ID 3");
    errno = 0;

    printf("Number of mailboxes: %ld", count_mbox_421_syscall());
    perror("Call count mb");
    errno = 0;

    printf("\n");

    //Reset at the end
    printf("Removing mailboxes...\n");
    reset_mbox_421_syscall();
    perror("Check remove");
    errno = 0;

    return 0;
}