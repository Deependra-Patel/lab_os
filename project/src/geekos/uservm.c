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
#include <geekos/ktypes.h>
#include <geekos/kassert.h>
#include <geekos/segment.h>
#include <geekos/gdt.h>
#include <geekos/paging.h>

extern Spin_Lock_t kthreadLock;
extern pde_t *g_kernel_pde;
uint_t user_vm_start = 1<<31;
#define USER_VM_START user_vm_start
#define USER_VM_LEN user_vm_start
#define DEFAULT_STACK_SIZE 2*PAGE_SIZE

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

struct User_Context * Create_User_Context(){
    struct User_Context * user_context;
    user_context=(struct User_Context *)Malloc(sizeof(struct User_Context));
    if(user_context==NULL){
        Print("malloc User_Context fail in Create_User_Context/n");
        return NULL;
    }
    user_context->ldtDescriptor=NULL;
    user_context->memory=NULL;
    user_context->size=0;
    user_context->ldtSelector=0;
    user_context->csSelector=0;
    user_context->dsSelector=0;

    user_context->pageDir=NULL;
    user_context->entryAddr=0;
    user_context->argBlockAddr=0;
    user_context->stackPointerAddr=0;
    user_context->refCount=0;
    return user_context;
}

void Destroy_User_Context(struct User_Context *userContext) {
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

uint_t lin_to_phyaddr(pde_t * page_dir, uint_t lin_address)
{
    uint_t pagedir_index=lin_address>>22;
    uint_t page_index=(lin_address<<10)>>22;
    uint_t offset_address=lin_address & 0xfff;

    Print("pgdir: %d\n",pagedir_index);
    pde_t * pagedir_entry=page_dir+pagedir_index;
    pte_t * page_entry=0;

    if(pagedir_entry->present){
        page_entry=(pte_t*) ((uint_t)pagedir_entry->pageTableBaseAddr << 12);
        page_entry+=page_index;
        //-----------------mydebug----------------------
        if(page_entry->present==0){
            Print("PGIND: %d\n", page_index);
            Print("the page is not present! /n");
            KASSERT(0);
        }
        //----------------------------------------------
        return (page_entry->pageBaseAddr << 12)+offset_address;
    }
    else {
        Print("Trying to resolve non-existent address%u/n", lin_address);
        return 0;
    }
}

// bool mydebug = false;

bool Copy_User_Page(pde_t * page_dir, uint_t dest_user, char * src, uint_t byte_num){
            Print("dest_user, %u, byte_num %d\n",dest_user, byte_num);     
    uint_t phy_start;
    uint_t temp_len;
    int page_nums;
    struct Page * page;

    if(Round_Down_To_Page(dest_user+byte_num) == Round_Down_To_Page(dest_user)) {
        temp_len=byte_num;
        page_nums=1;
    }
    else {
        temp_len=Round_Up_To_Page(dest_user)-dest_user;
        byte_num-=temp_len;
        page_nums=0;
    }

    //if (mydebug)  KASSERT(0);

    phy_start=lin_to_phyaddr(page_dir, dest_user);
    if(phy_start==0) {
        return false;
    }
    page = Get_Page(phy_start);
    Disable_Interrupts();
    page->flags &= ~ PAGE_PAGEABLE;
    Enable_Interrupts();
    memcpy((char *)phy_start, src, temp_len);
    page->flags |= PAGE_PAGEABLE;

    if(page_nums == 1) {
        return true;
    }

    temp_len=Round_Up_To_Page(dest_user)-dest_user;
    dest_user+=temp_len;
    src+=temp_len;
    byte_num-=temp_len;

    while(dest_user!=Round_Down_To_Page(dest_user + byte_num)) {
        phy_start=lin_to_phyaddr(page_dir, dest_user);
        if(phy_start == 0) {
            return false;
        }
        page = Get_Page(phy_start);
        Disable_Interrupts();
        page->flags &= ~ PAGE_PAGEABLE;
        Enable_Interrupts();

        memcpy((char*)phy_start, src, PAGE_SIZE);
        page->flags |= PAGE_PAGEABLE;

        dest_user+=PAGE_SIZE;
        byte_num-=PAGE_SIZE;
        src+=PAGE_SIZE;
    }

    phy_start = lin_to_phyaddr(page_dir, dest_user);
    if(phy_start==0) {
        return false;
    }
    Disable_Interrupts();
    page->flags &= ~ PAGE_PAGEABLE;
    Enable_Interrupts();

    memcpy((char*)phy_start, src, byte_num);
    page->flags |= PAGE_PAGEABLE;
    return true;
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
    // int f = 0;
    // while(command[f]!='\0'){
    //     Print("%c", command[f]);
    //     f++;
    // }
    // Print("Len: %d", (int)exeFileLength);
    // KASSERT(0);
    struct User_Context *uContext;
    uContext=Create_User_Context();

    uContext->ldtDescriptor=Allocate_Segment_Descriptor();
    if(uContext->ldtDescriptor==NULL){
        Print("allocate segment descriptor fail/n"); 
        return -1;
    }

    Init_LDT_Descriptor(uContext->ldtDescriptor, uContext->ldt, NUM_USER_LDT_ENTRIES);
    uContext->ldtSelector=Selector(USER_PRIVILEGE, true, Get_Descriptor_Index(uContext->ldtDescriptor));

    Init_Code_Segment_Descriptor(&uContext->ldt[0], USER_VM_START, USER_VM_LEN/PAGE_SIZE, USER_PRIVILEGE);
    Init_Data_Segment_Descriptor(&uContext->ldt[1], USER_VM_START, USER_VM_LEN/PAGE_SIZE, USER_PRIVILEGE);

    uContext->csSelector=Selector(USER_PRIVILEGE, false,0);
    uContext->dsSelector=Selector(USER_PRIVILEGE, false,1);

    //mydebug
    //Print("next will handle page data in Load_User_Program!/n");
    pde_t * pageDirectory;
    pageDirectory=(pde_t *)Alloc_Page();
    if(pageDirectory==NULL){
    //mydebug
        Print("no more page!/n");
        return -1;
    }
    memset(pageDirectory,'\0', PAGE_SIZE);
    memcpy(pageDirectory, g_kernel_pde, PAGE_SIZE);
    memset(pageDirectory+512,'\0', PAGE_SIZE/2);

//-------------------epic
    pde_t* cur_pde_entry = pageDirectory + 1019;
    pte_t * cur_pte;
    cur_pde_entry->present=1;
    cur_pde_entry->flags=VM_WRITE;
    cur_pte=(pte_t *)Alloc_Page();

    KASSERT(cur_pte!=NULL);

    uint_t mem_addr = 1019*NUM_PAGE_TABLE_ENTRIES*(long long)(PAGE_SIZE);
    memset(cur_pte,'\0',PAGE_SIZE);
    cur_pde_entry->pageTableBaseAddr=(uint_t)cur_pte>>12;
    int j;
    for(j = 0;j < PAGE_SIZE;j++){
        cur_pte->present=1;
        cur_pte->flags=VM_WRITE;
        cur_pte->pageBaseAddr=mem_addr>>12;
        cur_pte++;
        mem_addr+=PAGE_SIZE;
    }
//----------------apic holeee ---------------
    uContext->pageDir=pageDirectory;
    Print("pdir: %u\n", (uint_t)pageDirectory);
    // KASSERT(0);
    int i;
    int res;
    uint_t startAddress=0;
    uint_t sizeInMemory=0;
    uint_t offsetInFile=0;
    uint_t lengthInFile=0;
    Print("numSeg: %d\n", exeFormat->numSegments);
    for(i=0; i<exeFormat->numSegments; i++){
        startAddress = exeFormat->segmentList[i].startAddress;
        sizeInMemory = exeFormat->segmentList[i].sizeInMemory;

        offsetInFile = exeFormat->segmentList[i].offsetInFile;
        lengthInFile = exeFormat->segmentList[i].lengthInFile;

        if(startAddress + sizeInMemory < USER_VM_LEN){
            Print("%d\n", startAddress+USER_VM_START);
            res=Alloc_User_Page(pageDirectory, startAddress+USER_VM_START, sizeInMemory);
            if(res!=0){
                return -1;
            }
            if ((sizeInMemory == 0) && (lengthInFile == 0)) continue;
            Print("sizeinmem, %u leninfile: %d\n", sizeInMemory, lengthInFile);
            res=Copy_User_Page(pageDirectory, startAddress+USER_VM_START, exeFileData+offsetInFile, lengthInFile);
            if(res!=true){
                return -1;
            }
        } 
        else{
            Print("startAddress+sizeInMemory > 2GB in Load_User_Program/n");
            return -1;
        }
    }

    //KASSERT(0);
    //mydebug
    //Print("next will handle block an stack in Load_User_Program!/n");
    Print("Converted: %d\n", ((int)(((pte_t*)(pageDirectory[512].pageTableBaseAddr<<12))->pageBaseAddr<<12)));
    uint_t args_num;
    uint_t stack_addr;
    uint_t arg_addr;
    ulong_t arg_size;
    Get_Argument_Block_Size(command, &args_num, &arg_size);
    if(arg_size > PAGE_SIZE){
        Print("Argument Block too big for one PAGE_SIZE/n");
        return -1;
    }
    arg_addr=Round_Down_To_Page(USER_VM_LEN-arg_size);
    char* block_buffer=Malloc(arg_size);
    KASSERT(block_buffer!=NULL);
    Format_Argument_Block(block_buffer, args_num, arg_addr, command);

    res=Alloc_User_Page(pageDirectory, arg_addr+USER_VM_START, arg_size);
    if(res!=0){
        return -1;
    }

    res=Copy_User_Page(pageDirectory, arg_addr+USER_VM_START, block_buffer, arg_size);
    if(res!=true){
        return -1;
    }

    Free(block_buffer);

    stack_addr=USER_VM_LEN-Round_Up_To_Page(arg_size)-DEFAULT_STACK_SIZE;

    res=Alloc_User_Page(pageDirectory, stack_addr+USER_VM_START, DEFAULT_STACK_SIZE);

    if(res!=0){
        return -1;
    }

    uContext->entryAddr = exeFormat->entryAddr;
    //Print("Setting entry address to %lu/n", (*pUserContext)->entryAddr);
    uContext->argBlockAddr = arg_addr;
    uContext->size = USER_VM_LEN;
    uContext->stackPointerAddr = arg_addr;
    *pUserContext=uContext;
    //mydebug
    //Print("Load_User_Program will return!/n");
    return 0;
}

/*
 * Copy data from user buffer into kernel buffer.
 * Returns true if successful, false otherwise.
 */
bool Copy_From_User(void *destInKernel, ulong_t srcInUser, ulong_t numBytes) {

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
    struct User_Context* userContext=CURRENT_THREAD->userContext;
    void* user_lin_addr=(void*)(USER_VM_START)+srcInUser;

    if((srcInUser+numBytes) < userContext->size){
        memcpy(destInKernel, user_lin_addr, numBytes);
        return true;
    }
    return false;
   // TODO_P(PROJECT_VIRTUAL_MEMORY_A, "Copy user data to kernel buffer");
}

/*
 * Copy data from kernel buffer into user buffer.
 * Returns true if successful, false otherwise.
 */
bool Copy_To_User(ulong_t destInUser, void *srcInKernel, ulong_t numBytes) {

    /*
     * Hints:
     * - Same as for Copy_From_User()
     * - Also, make sure the memory is mapped into the user
     *   address space with write permission enabled
     */
    struct User_Context* userContext=CURRENT_THREAD->userContext;
    void* user_lin_addr = (void*)(USER_VM_START) + destInUser;
    if((destInUser+numBytes) < userContext->size){
        memcpy(user_lin_addr, srcInKernel ,numBytes);
        return true;
    }
    return false;     
    //TODO_P(PROJECT_VIRTUAL_MEMORY_A, "Copy kernel data to user buffer");
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
extern int Load_LDTR(ushort_t);
void Switch_To_Address_Space(struct User_Context *userContext) {
    // KASSERT(0);

    /*
     * - If you are still using an LDT to define your user code and data
     *   segments, switch to the process's LDT
     * - 
     */
    if(userContext == 0){
        Print("the userContext is NULL!/n");
        KASSERT(0);
        return;
    }
        Set_PDBR(userContext->pageDir);

    Load_LDTR(userContext->ldtSelector);



    // TODO_P(PROJECT_VIRTUAL_MEMORY_A,
    //        "Switch_To_Address_Space() using paging");
}
