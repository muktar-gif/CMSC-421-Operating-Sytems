#include <linux/kernel.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define __NR_create_mbox_421 436
#define __NR_remove_mbox_421 437
#define __NR_reset_mbox_421 438
#define __NR_count_mbox_421 439
#define __NR_list_mbox_421 440
#define __NR_send_msg_421 441
#define __NR_recv_msg_421 442
#define __NR_peek_msg_421 443
#define __NR_count_msg_421 444
#define __NR_len_msg_421 445
#define __NR_print_mbox_421 446


long create_mbox_421_syscall(unsigned long id) {

    return syscall(__NR_create_mbox_421, id);

}

long remove_mbox_421_syscall(unsigned long id) {

    return syscall(__NR_remove_mbox_421, id);

}

long reset_mbox_421_syscall(void) {

    return syscall(__NR_reset_mbox_421);

}

long count_mbox_421_syscall(void) {

    return syscall(__NR_count_mbox_421);

}

long list_mbox_421_syscall(unsigned long *mbxes, long k){

    return syscall(__NR_list_mbox_421, mbxes, k);

}

long send_msg_421_syscall(unsigned long id, const unsigned char *msg, long n){

    return syscall(__NR_send_msg_421, id, msg, n);

}

long recv_msg_421_syscall(unsigned long id, unsigned char *msg, long n){

    return syscall(__NR_recv_msg_421, id, msg, n);

}

long peek_msg_421_syscall(unsigned long id, unsigned char *msg, long n) {

    return syscall(__NR_peek_msg_421, id, msg, n);

}

long count_msg_421_syscall(unsigned long id) {

    return syscall(__NR_count_msg_421, id);

}

long len_msg_421_syscall(unsigned long id) {

    return syscall(__NR_len_msg_421, id);

}

long print_mbox_421_syscall(unsigned long id) {

    return syscall(__NR_print_mbox_421, id);

}


