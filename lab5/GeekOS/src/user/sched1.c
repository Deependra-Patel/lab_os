/*
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003,2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 */
#include <conio.h>
#include <process.h>
#include <sched.h>
#include <sema.h>
#include <string.h>

int main(int argc __attribute__ ((unused)), char **argv
         __attribute__ ((unused))) {

    int i, j;                   /* loop index */
    int holdsched3_sem;

    holdsched3_sem = Open_Semaphore("holdsched3_sem", 0);

    for (i = 0; i < 10; i++) {
        for (j = 0; j < 20000; j++);
        Print("1");
    }

    V(holdsched3_sem);

    for (i = 0; i < 10; i++) {
        for (j = 0; j < 20000; j++);
        Print("1");
    }

    Close_Semaphore(holdsched3_sem);

    return 0;
}
