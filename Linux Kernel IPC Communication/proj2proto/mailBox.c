#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


#include "list.h"

long create_mbox_421(unsigned long id);
long remove_mbox_421(unsigned long id);
long reset_mbox_421(void);

long count_mbox_421();

long list_mbox_421(unsigned long  *mbxes, long k); // __user
long send_msg_421(unsigned long id, const unsigned char  *msg, long n); //__user
long recv_msg_421(unsigned long id, unsigned char *msg, long n); //__user
long peek_msg_421(unsigned long id, unsigned char *msg, long n); //__user

long count_msg_421(unsigned long id); 
long len_msg_421(unsigned long id); 
long print_mbox_421(unsigned long id);

/*message list for mailbox*/
typedef struct message{

    char  *info;
    unsigned long numBytes;
    struct list_head messageHead;

} message_t;

/*list for mailbox*/
typedef struct mailbox {

    unsigned long mailboxId;
    unsigned long numMessages;

    struct list_head mailboxHead;
    struct list_head messages;

} mailbox_t;

LIST_HEAD(mailboxes);

int main(void){

    //testing mailbox
    //Trying to reset without making mailboxes
    reset_mbox_421();
    
    for (int i = 0; i < 4; i++) {    
        create_mbox_421(i);
    }

    printf("%d\n", create_mbox_421(20));
        
    //send_msg_421(0,"Hello ID 0", 5);
    //send_msg_421(0,"Hello2", 2);

    //send_msg_421(1, "what", 3);
    
    char *test = "Hello ID 2";
    send_msg_421(2,test, 10);

    /*
    send_msg_421(2,"Hello Test ID 2", 15);
    send_msg_421(2,"Message", 7);
    */

    /*
    send_msg_421(3,"Hello ID 3", 10);
    send_msg_421(3,"Hello New Message ID 3", 22);
    send_msg_421(3,"Goodbye", 7);
    */

    printf("Number of MailBoxes: %d\n", count_mbox_421());

    printf("Length of 2: %d\n", len_msg_421(2));
    printf("Length of 0: %d\n", len_msg_421(0));
    printf("Length of 1: %d\n", len_msg_421(1));

    //printf("%d\n", len_msg_421(2));
    //printf("%d\n", len_msg_421(2));
    //printf("%d\n", len_msg_421(2));

    //printf("Length of message %d\n", len_msg_421(0));

    int bytes = 0;

    char *string = "hello";

    bytes = send_msg_421(0,"hello", 5);
    printf("Bytes stored: %d\n", bytes);

    char *msg = (char*)malloc(5); 
    bytes = recv_msg_421(0, msg, 5);
    printf("Bytes received: %d\n", bytes);

    if (bytes != -1)
        printf("Printing message 0: %s\n", msg);
    

    free(msg);
    
    long *mboxes = (long*)malloc(sizeof(long)*5);
    long copied = list_mbox_421(mboxes, 5);

    for (int i = 0; i < copied; i++){

        printf("ID: %d\n", mboxes[i]);

    }

    free(mboxes);
    /*
    printf("%d\n", copied);

    free(mboxes);

    printf("Printing MailBox ID 0 : \n");
    //print_mbox_421(0);

    /*
    char *msg1 = (char*)malloc(10);
    recv_msg_421(2, msg1, 1);
    free(msg1);
    char *msg2 = (char*)malloc(10);
    recv_msg_421(2, msg2, 1);
    free(msg2);
    char *msg3 = (char*)malloc(10);
    recv_msg_421(2, msg3, 1);
    free(msg3);
    */
    
   /*
    printf("Trying to remove 2 with messages %d\n", remove_mbox_421(2));

    printf("Number of messages of 2: %d\n", count_msg_421(2));
    
    struct list_head *curr;
    list_for_each(curr, &mailboxes){

        mailbox_t *getMailBox = NULL;
        getMailBox = list_entry(curr, mailbox_t, mailboxHead);

        printf("%d", getMailBox -> mailboxId);

    }


    printf("\nNumber of MB: %d\n", count_mbox_421());

    reset_mbox_421();

    printf("\nNumber of MB: %d\n", count_mbox_421());

    for (int i = 0; i < 4; i++) {    
        create_mbox_421(i);
    }

    printf("\nNumber of MB: %d\n", count_mbox_421());
    */
    reset_mbox_421();


    return 0;
}

/*creates an empty mailbox for the list*/
long create_mbox_421(unsigned long id){

    
    struct list_head *curr;
    list_for_each(curr, &mailboxes){

        mailbox_t *currMailbox = NULL;
        currMailbox = list_entry(curr, mailbox_t, mailboxHead);

        /*if the id is found*/
        if (currMailbox -> mailboxId == id) {
            
            /*id already exists*/
            return -1;
            
        }

    }

    /*creates a mailbox and adds it to list*/
    mailbox_t *newMailbox = (mailbox_t*) malloc(sizeof(mailbox_t));
    newMailbox -> mailboxId = id;
    newMailbox -> numMessages = 0;

    //initalize head
    INIT_LIST_HEAD(&newMailbox -> mailboxHead);
    INIT_LIST_HEAD(&newMailbox -> messages);
    list_add_tail(&newMailbox -> mailboxHead, &mailboxes);

    return 0;
    
}

/*removes mailbox were id is found*/
long remove_mbox_421(unsigned long id){

    struct list_head *curr = NULL;
    struct list_head *temp;

    list_for_each_safe(curr, temp, &mailboxes){

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        /*removes mailbox if id was found*/
        if (getMailbox -> mailboxId == id){
            
            if (getMailbox -> numMessages != 0) {

                return -1;
            
            }
            else {
                
                list_del(curr);

                free(getMailbox);
                return 0;
            }

        }

    }

    /*cannot find id*/
    return -1;
    
}

long reset_mbox_421(void) {

    mailbox_t *currMailbox;
    mailbox_t *tempMailbox;

    /*loops through mailbox*/
    list_for_each_entry_safe(currMailbox, tempMailbox, &mailboxes, mailboxHead){

        message_t *currMessage;
        message_t *tempMessage;

        /*loops through messages*/
        list_for_each_entry_safe(currMessage, tempMessage, &currMailbox -> messages, messageHead){

            free(currMessage -> info);
            list_del(&currMessage -> messageHead);
            free(currMessage);
        }

        list_del(&currMailbox -> mailboxHead);
        free(currMailbox);

    }

    return 0;
}

/*counts the number of mailboxes in the list*/
long count_mbox_421(void) {

    struct list_head *currMailbox;
    int count = 0;

    //loops mailbox and counts
    list_for_each(currMailbox, &mailboxes) {
                
        mailbox_t *tempMailbox = NULL;
        tempMailbox = list_entry(currMailbox, mailbox_t, mailboxHead);

        count++;
    }

    return count;
}

long list_mbox_421(unsigned long *mbxes, long k){ // __user

    struct list_head *curr = NULL;

    int countList = 0;
    list_for_each(curr, &mailboxes) {

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        if (countList == k) {

            return countList;
            
        }
        else {

            mbxes[countList] = getMailbox -> mailboxId;
            countList++;
        }

    }

    //failed
    return countList;
    

}

long send_msg_421(unsigned long id, const unsigned char  *msg, long n) { //__user

    struct list_head *curr = NULL;

    list_for_each(curr, &mailboxes){

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        //id is found to send message
        if (getMailbox -> mailboxId == id){
            
            if (n < 0) {

                fprintf(stderr, "Negative length not allowed\n"); 

                //failed
                return -1;

            } 

            message_t *createMessage = (message_t*)malloc(sizeof(message_t));

            createMessage -> info = (char*) malloc(n + 1);

            for (int i = 0; i < n; i++){

                createMessage -> info[i] = msg[i];

            }

            createMessage -> info[n] = '\0';
            
            list_add_tail(&createMessage -> messageHead, &getMailbox -> messages);

            getMailbox -> numMessages++;

            return n;
        }

    }

    fprintf(stderr, "Failed to find mailbox ID\n");

    //failed
    return -1;

}


long recv_msg_421(unsigned long id, unsigned char  *msg, long n) { //__user

    
    struct list_head *curr = NULL;
    struct list_head *temp;

    list_for_each_safe(curr, temp, &mailboxes){

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        //id found
        if (getMailbox -> mailboxId == id) {

            message_t *getMessage = list_first_entry(&getMailbox -> messages ,message_t, messageHead);

            if (list_empty(&getMessage -> messageHead)){
                
                return -1;

            }

            int copySize = 0;
            for (int i = 0; i < n; i++){
                
                if (getMessage -> info[i] == '\0') {
                    copySize = i;
                    i = n;
                }
                else
                    msg[i] = getMessage -> info[i];

            }

            msg[copySize] = '\0';

            free(getMessage -> info);
            list_del(&getMessage -> messageHead);
            free(getMessage);

            getMailbox -> numMessages--;

            return copySize;
        }

    }

    fprintf(stderr, "Failed to find message\n");
    return -1;


}


long peek_msg_421(unsigned long id, unsigned char *msg, long n) { //__user 

    struct list_head *curr = NULL;
    

    list_for_each(curr, &mailboxes){

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        /*id found*/
        if (getMailbox -> mailboxId == id) {

            message_t *getMessage = list_first_entry(&getMailbox -> messages ,message_t, messageHead);
            
            if (list_empty(&getMessage -> messageHead)){

                return -1;

            }

            int copySize = 0;
            for (int i = 0; i < n; i++){
                
                if (getMessage -> info[i] == '\0') {
                    copySize = i;
                    i = n;
                }
                else
                    msg[i] = getMessage -> info[i];

            }

            msg[copySize] = '\0';

            return copySize;
        }

    }

    fprintf(stderr, "Failed to find message\n");
    return -1;

}

long count_msg_421(unsigned long id) {

    struct list_head *currMailbox;

    list_for_each(currMailbox, &mailboxes) {
                
        mailbox_t *tempMailbox = NULL;
        tempMailbox = list_entry(currMailbox, mailbox_t, mailboxHead);

        if (tempMailbox -> mailboxId == id)
            return tempMailbox -> numMessages;
    }

    fprintf(stderr, "Failed to find message id\n");
    return -1;

}

long len_msg_421(unsigned long id) {

    struct list_head *curr = NULL;

    list_for_each(curr, &mailboxes){

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        /*id found*/
        if (getMailbox -> mailboxId == id) {

            message_t *getMessage = list_first_entry(&getMailbox -> messages ,message_t, messageHead);
            
            if (list_empty(&getMessage -> messageHead)){

                //error
                return -1;

            }

            int count = getMessage -> numBytes;

            
            while (getMessage -> info[count] != '\0') {
                count++;
            }
            

            if (count  == 0)
                fprintf(stderr, "No Messages\n");

            return count;
        }

    }

    fprintf(stderr, "Failed to find message ID\n");
    return -1;

}

/*prints messages with the given id*/
long print_mbox_421(unsigned long id){

    struct list_head *curr;

    list_for_each(curr, &mailboxes){

        mailbox_t *currMailbox = NULL;
        currMailbox = list_entry(curr, mailbox_t, mailboxHead);
        

        /*if the id is found*/
        if (currMailbox -> mailboxId == id) {
            
            message_t *currMessage;

            //loops through messages
            list_for_each_entry(currMessage, &currMailbox -> messages, messageHead) {
                
                //print with printk() newline every 16 bytes, not done
                
                int index  = 0;

                while (currMessage -> info[index] != '\0'){
                    
                    
                    printf("%02x ", currMessage -> info[index]);
                    if ((index + 1) % 16 == 0)
                        printf("\n");
                    
                    index++;
                }

                printf("\n---\n");
            }   

            printf("\n");
            return 0;
        }

    }

    return -1;

}


