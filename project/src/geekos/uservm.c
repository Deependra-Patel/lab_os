/*
 * Paging-based user mode implementation
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
 *
 * $Revision: 1.51 $
 */

#include <geekos/int.h>
#include <geekos/mem.h>
#include <geekos/paging.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/argblock.h>
#include <geekos/kthread.h>
#include <geekos/range.h>
#include <geekos/vfs.h>
#include <geekos/user.h>
#include <geekos/projects.h>


extern Spin_Lock_t kthreadLock;

int userDebug = 0;
#define Debug(args...) if (userDebug) Print("uservm: " args)

/* ----------------------------------------------------------------------
 * Private functions
 * ---------------------------------------------------------------------- */

// TODO: Add private functions


/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

/*
 * Destroy a User_Context object, including all memory
 * and other resources allocated within it.
 */
void Destroy_User_Context(struct User_Context *context) {
    KASSERT(0);
    /*
     * Hints:
     * - Free all pages, page tables, and page directory for
     *   the process (interrupts must be disabled while you do this,
     *   otherwise those pages could be stolen by other processes)
     * - Free semaphores, files, and other resources used
     *   by the process
     */
    KASSERT(userContext->refCount == 0);

    /* Free the context's LDT descriptor */
    Free_Segment_Descriptor(userContext->ldtDescriptor);

    /* Free the context's memory */
    bool iflag;
    iflag = Begin_Int_Atomic();
    //--destroy page table, page dir
    //--free all pages, also in swap space
    Free(userContext->memory);
    Free(userContext);
    End_Int_Atomic(iflag);

    TODO_P(PROJECT_VIRTUAL_MEMORY_A,
           "Destroy User_Context data structure after process exits");
}

/*
 * Load a user executable into memory by creating a User_Context
 * data structure.
 * Params:
 * exeFileData - a buffer containing the executable to load
 * exeFileLength - number of bytes in exeFileData
 * exeFormat - parsed ELF segment information describing how to
 *   load the executable's text and data segments, and the
 *   code entry point address
 * command - string containing the complete command to be executed:
 *   this should be used to create the argument block for the
 *   process
 * pUserContext - reference to the pointer where the User_Context
 *   should be stored
 *
 * Returns:
 *   0 if successful, or an error code (< 0) if unsuccessful
 */
int Load_User_Program(char *exeFileData, ulong_t exeFileLength,
                      struct Exe_Format *exeFormat, const char *command,
                      struct User_Context **pUserContext) {
    KASSERT(0); 
    /*
     * Hints:
     * - This will be similar to the same function in userseg.c
     * - Determine space requirements for code, data, argument block,
     *   and stack
     * - Allocate pages for above, map them into user address
     *   space (allocating page directory and page tables as needed)
     * - Fill in initial stack pointer, argument block address,
     *   and code entry point fields in User_Context
     */

    int i;
    ulong_t maxva = 0;
    unsigned numArgs;
    ulong_t argBlockSize;
    ulong_t size, argBlockAddr;
    struct User_Context *userContext = 0;

    /* Find maximum virtual address */
    for (i = 0; i < exeFormat->numSegments; ++i) {//block use here
        struct Exe_Segment *segment = &exeFormat->segmentList[i];
        ulong_t topva = segment->startAddress + segment->sizeInMemory;  /* FIXME: range check */

        if (topva > maxva)
            maxva = topva;
    }

    /* Determine size required for argument block */
    Get_Argument_Block_Size(command, &numArgs, &argBlockSize);

    /*
     * Now we can determine the size of the memory block needed
     * to run the process.
     */
    size = Round_Up_To_Page(maxva) + DEFAULT_USER_STACK_SIZE;
    argBlockAddr = size;
    size += argBlockSize;

    /* Create User_Context */
    userContext = Create_User_Context(size);
    if (userContext == 0)
        return -1;

    /* Load segment data into memory */
    for (i = 0; i < exeFormat->numSegments; ++i) {
        struct Exe_Segment *segment = &exeFormat->segmentList[i];

        memcpy(userContext->memory + segment->startAddress,
               exeFileData + segment->offsetInFile, segment->lengthInFile);
    }

    /* Format argument block */
    Format_Argument_Block(userContext->memory + argBlockAddr, numArgs,
                          argBlockAddr, command);

    /* Fill in code entry point */
    userContext->entryAddr = exeFormat->entryAddr;

    /*
     * Fill in addresses of argument block and stack
     * (They happen to be the same)
     */
    userContext->argBlockAddr = argBlockAddr;
    userContext->stackPointerAddr = argBlockAddr;


    *pUserContext = userContext;
    return 0;
    TODO_P(PROJECT_VIRTUAL_MEMORY_A, "Load user program into address space");
    return 0;
}

/*
 * Copy data from user buffer into kernel buffer.
 * Returns true if successful, false otherwise.
 */
bool Copy_From_User(void *destInKernel, ulong_t srcInUser, ulong_t numBytes) {
    KASSERT(0);

    /*
     * Hints:
     * - Make sure that user page is part of a valid region
     *   of memory
     * - Remember that you need to add 0x80000000 to user addresses
     *   to convert them to kernel addresses, because of how the
     *   user code and data segments are defined
     * - User pages may need to be paged in from disk before being accessed.
     * - Before you touch (read or write) any data in a user
     *   page, **disable the PAGE_PAGEABLE bit**.
     *
     * Be very careful with race conditions in reading a page from disk.
     * Kernel code must always assume that if the struct Page for
     * a page of memory has the PAGE_PAGEABLE bit set,
     * IT CAN BE STOLEN AT ANY TIME.  The only exception is if
     * interrupts are disabled; because no other process can run,
     * the page is guaranteed not to be stolen.
     */
    TODO_P(PROJECT_VIRTUAL_MEMORY_A, "Copy user data to kernel buffer");
}

/*
 * Copy data from kernel buffer into user buffer.
 * Returns true if successful, false otherwise.
 */
bool Copy_To_User(ulong_t destInUser, void *srcInKernel, ulong_t numBytes) {
    KASSERT(0);

    /*
     * Hints:
     * - Same as for Copy_From_User()
     * - Also, make sure the memory is mapped into the user
     *   address space with write permission enabled
     */
    TODO_P(PROJECT_VIRTUAL_MEMORY_A, "Copy kernel data to user buffer");
}

/*
 * Copy data from user buffer into user buffer.
 * Returns true if successful, false otherwise.
 */
bool Copy_User_To_User(void *destInUser, ulong_t srcInUser, ulong_t numBytes) {
    KASSERT(0);

    /*
     * Hints:
     * - Make sure that each user page is part of a valid region
     *   of memory
     * - Remember that you need to add 0x80000000 to user addresses
     *   to convert them to kernel addresses, because of how the
     *   user code and data segments are defined
     * - User pages may need to be paged in from disk before being accessed.
     * - Before you touch (read or write) any data in a user
     *   page, **disable the PAGE_PAGEABLE bit**.
     *
     * Be very careful with race conditions in reading a page from disk.
     * Kernel code must always assume that if the struct Page for
     * a page of memory has the PAGE_PAGEABLE bit set,
     * IT CAN BE STOLEN AT ANY TIME.  The only exception is if
     * the vmSpingLock is held; because no other process can run,
     * the page is guaranteed not to be stolen.
     */
    TODO_P(PROJECT_VIRTUAL_MEMORY_A, "Copy user data to kernel buffer");
}

/*
 * Switch to user address space.
 */
void Switch_To_Address_Space(struct User_Context *userContext) {
    KASSERT(0);

    /*
     * - If you are still using an LDT to define your user code and data
     *   segments, switch to the process's LDT
     * - 
     */
    TODO_P(PROJECT_VIRTUAL_MEMORY_A,
           "Switch_To_Address_Space() using paging");
}
