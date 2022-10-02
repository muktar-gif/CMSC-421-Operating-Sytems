#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/printk.h>
#include <linux/errno.h>


/*node for message list*/
typedef struct message{

    char  *info;
    unsigned long messageLength;
    struct list_head messageHead;

} message_t;

/*node for mailbox*/
typedef struct mailbox {

    unsigned long mailboxId;
    unsigned long numMessages;

    struct list_head mailboxHead;
    struct list_head messages;

} mailbox_t;

LIST_HEAD(mailboxes); 

SYSCALL_DEFINE1(create_mbox_421, unsigned long, id) {

    /*checks restrictions*/
    kuid_t getRoot;

    getRoot.val = 0;
    if(!uid_eq(get_current_cred() -> uid, getRoot))
        return -EPERM;

    struct list_head *curr;
    list_for_each(curr, &mailboxes){

        mailbox_t *currMailbox = NULL;
        currMailbox = list_entry(curr, mailbox_t, mailboxHead);

        /*if the id is found*/
        if (currMailbox -> mailboxId == id) {
            
            /*id already exists*/
            return -EEXIST;
            
        }

    }

    /*creates a mailbox and adds it to list*/
    mailbox_t *newMailbox = (mailbox_t*) kmalloc(sizeof(mailbox_t), GFP_KERNEL);

    if (newMailbox == NULL) {

        return -ENOMEM;

    }

    newMailbox -> mailboxId = id;
    newMailbox -> numMessages = 0;

    /*initalize head*/
    INIT_LIST_HEAD(&newMailbox -> mailboxHead);
    INIT_LIST_HEAD(&newMailbox -> messages);
    list_add_tail(&newMailbox -> mailboxHead, &mailboxes);

    return 0;
}

SYSCALL_DEFINE1(remove_mbox_421, unsigned long, id) {

    /*checks restrictions*/
    kuid_t getRoot;

    getRoot.val = 0;
    if(!uid_eq(get_current_cred() -> uid, getRoot))
        return -EPERM;

    struct list_head *curr = NULL;
    struct list_head *temp;

    list_for_each_safe(curr, temp, &mailboxes){

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        /*removes mailbox if id was found*/
        if (getMailbox -> mailboxId == id){
            
            /*if there are still messages in mailbox*/
            if (getMailbox -> numMessages != 0) {

                return -ENOTEMPTY;
            
            }
            else {
                
                list_del(curr);

                kfree(getMailbox);
                return 0;
            }
        }
    }

    /*cannot find id*/
    return -ENOENT;
}

SYSCALL_DEFINE0(reset_mbox_421) {

    /*checks restrictions*/
    kuid_t getRoot;

    getRoot.val = 0;
    if(!uid_eq(get_current_cred() -> uid, getRoot))
        return -EPERM;
    
    mailbox_t *currMailbox;
    mailbox_t *tempMailbox;

    /*loops through mailbox*/
    list_for_each_entry_safe(currMailbox, tempMailbox, &mailboxes, mailboxHead){

        message_t *currMessage;
        message_t *tempMessage;

        /*loops through messages*/
        list_for_each_entry_safe(currMessage, tempMessage, &currMailbox -> messages, messageHead){

            /*kfreeing information and message*/
            kfree(currMessage -> info);
            list_del(&currMessage -> messageHead);
            kfree(currMessage);

        }

        /*kfreeing mailbox*/
        list_del(&currMailbox -> mailboxHead);
        kfree(currMailbox);

    }

    return 0;
}

SYSCALL_DEFINE0(count_mbox_421) {

    struct list_head *currMailbox;
    int count = 0;

    /*loops mailbox and counts*/
    list_for_each(currMailbox, &mailboxes) {
                
        mailbox_t *tempMailbox = NULL;
        tempMailbox = list_entry(currMailbox, mailbox_t, mailboxHead);

        count++;
    }

    return count;
    
}

SYSCALL_DEFINE2(list_mbox_421, unsigned long __user *, mxbes, long, k) {

    if (k < 0)
        return -ENOENT;

    /*validates pointer*/
    if (!access_ok(mxbes, k))
        return -EFAULT;

    unsigned long *makeMxbes = (long*) kmalloc(sizeof(unsigned long) * k, GFP_KERNEL); 

    if (makeMxbes == NULL)
        return -ENOMEM;
    
    struct list_head *curr = NULL;

    int countList = 0;
    list_for_each(curr, &mailboxes) {

        mailbox_t *getMailbox = NULL;
        getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        if (countList < k) {

            makeMxbes[countList] = getMailbox -> mailboxId;
            countList++;
        
        }

    }

    /*copies pointer*/
    if (copy_to_user(mxbes, makeMxbes, sizeof(unsigned long) * countList) != 0)
        return -EFAULT;

    kfree(makeMxbes);

    /*return number of bytes compiled*/
    return countList;
}

SYSCALL_DEFINE3(send_msg_421, unsigned long, id, const unsigned char __user *, msg, long, n) {

    /*check n negative*/
    if (n < 0)
        return -ENOENT;

    /*checks valid pointer then copies pointer to copy message*/
    if (!access_ok(msg, n))
        return -EFAULT;

    struct list_head *curr = NULL;

    list_for_each(curr, &mailboxes) {

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        /*id is found to send message*/
        if (getMailbox -> mailboxId == id){
            
            /*allocate memory to new and check ptr*/
            message_t *createMessage = (message_t*)kmalloc(sizeof(message_t), GFP_KERNEL);
            
            /*if memory could not be allocated*/
            if (createMessage == NULL) 
                return -ENOMEM;

            /*allocate memory for actual message*/
            createMessage -> info = (char*) kmalloc(n, GFP_KERNEL);

            /*if memory could not be allocated*/
            if (createMessage -> info == NULL) {
                
                kfree(createMessage);
                kfree(createMessage -> info);
                return -ENOMEM;
        
            }
            
            /*copies message to mailbox message*/
            if (copy_from_user(createMessage -> info, msg, n) != 0) {

                kfree(createMessage);
                kfree(createMessage -> info);
                return -EFAULT;

            }

            createMessage -> messageLength = n;

            /*adds message*/
            list_add_tail(&createMessage -> messageHead, &getMailbox -> messages);
            getMailbox -> numMessages++;

            return n;
        }

    }

    /*cannot find id*/
    return -ENOENT;
                               
}

SYSCALL_DEFINE3(recv_msg_421, unsigned long, id, unsigned char __user *, msg, long, n) {

    /*check n negative*/
    if (n < 0)
        return -ENOENT;

    /*validates pointer*/
    if (!access_ok(msg, n))
        return -EFAULT;

    struct list_head *curr = NULL;
    struct list_head *temp;

    list_for_each_safe(curr, temp, &mailboxes){

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        /*id found*/
        if (getMailbox -> mailboxId == id) {

            message_t *getMessage = list_first_entry(&getMailbox -> messages ,message_t, messageHead);

            /*if no messages*/
            if (list_empty(&getMessage -> messageHead))
                return -ENOENT;

            unsigned long size;

            if (getMessage -> messageLength < n){

                size = getMessage -> messageLength;

            }
            else {

                size = n;
                
            }

            /*copies pointer*/
            if (copy_to_user(msg, getMessage -> info, size) != 0)
                return -EFAULT;

            kfree(getMessage -> info);
            list_del(&getMessage -> messageHead);
            kfree(getMessage);

            getMailbox -> numMessages--;

            return size;
        }

    }

    /*cannot find id*/
    return -ENOENT;
    
}

SYSCALL_DEFINE3(peek_msg_421, unsigned long, id, unsigned char __user *, msg, long, n) {

    /*check n negative*/
    if (n < 0)
        return -ENOENT;

    /*validates pointer*/
    if (!access_ok(msg, n))
        return -EFAULT;

    struct list_head *curr = NULL;
    struct list_head *temp;

    list_for_each_safe(curr, temp, &mailboxes){

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        /*id found*/
        if (getMailbox -> mailboxId == id) {

            message_t *getMessage = list_first_entry(&getMailbox -> messages ,message_t, messageHead);

            /*if no messages*/
            if (list_empty(&getMessage -> messageHead))
                return -ENOENT;

            unsigned long size;
            if (getMessage -> messageLength < n){

                size = getMessage -> messageLength;

            }
            else {

                size = n;
                
            }

            /*copies pointer*/
            if (copy_to_user(msg, getMessage -> info, size) != 0)
                return -EFAULT;

            return size;
        }

    }

    /*cannot find id*/
    return -ENOENT;
    
}

SYSCALL_DEFINE1(count_msg_421, unsigned long, id) {

    struct list_head *currMailbox;

    /*loops through mailbox and finds num messages*/
    list_for_each(currMailbox, &mailboxes) {
                
        mailbox_t *tempMailbox = NULL;
        tempMailbox = list_entry(currMailbox, mailbox_t, mailboxHead);

        if (tempMailbox -> mailboxId == id)
            return tempMailbox -> numMessages;
    }

    /*cannot find id*/
    return -ENOENT;
    
}

SYSCALL_DEFINE1(len_msg_421, unsigned long, id) {

    struct list_head *curr = NULL;

    list_for_each(curr, &mailboxes) {

        mailbox_t *getMailbox = list_entry(curr, mailbox_t, mailboxHead);

        /*id found*/
        if (getMailbox -> mailboxId == id) {

            message_t *getMessage = list_first_entry(&getMailbox -> messages ,message_t, messageHead);
            
            /*if no messages*/
            if (list_empty(&getMessage -> messageHead)) 
                return -ENOENT;


            return getMessage -> messageLength;
        }

    }

    /*cannot find id*/
    return -ENOENT;
    
}

SYSCALL_DEFINE1(print_mbox_421, unsigned long, id) {

    struct list_head *curr;

    list_for_each(curr, &mailboxes){

        mailbox_t *currMailbox = NULL;
        currMailbox = list_entry(curr, mailbox_t, mailboxHead);
        

        /*if the id is found*/
        if (currMailbox -> mailboxId == id) {
            
            message_t *currMessage;

            /*loops through messages and prints*/
            list_for_each_entry(currMessage, &currMailbox -> messages, messageHead) {
                
                int index;
                for(index = 0; index < currMessage -> messageLength; index++) {
                    
                    printk("%02x ", currMessage -> info[index]);
                    if ((index + 1) % 16 == 0)
                        printk("\n");
                    
                }

                printk("\n---\n");

            }   

            printk("\n");
            return 0;
        }

    }

    /*cannot find id*/
    return -ENOENT;
    
}

