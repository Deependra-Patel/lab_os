/*
 * Paging (virtual memory) support
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
 * $Revision: 1.56 $
 * 
 */

#include <geekos/string.h>
#include <geekos/int.h>
#include <geekos/idt.h>
#include <geekos/kthread.h>
#include <geekos/kassert.h>
#include <geekos/screen.h>
#include <geekos/mem.h>
#include <geekos/malloc.h>
#include <geekos/gdt.h>
#include <geekos/segment.h>
#include <geekos/user.h>
#include <geekos/vfs.h>
#include <geekos/crc32.h>
#include <geekos/paging.h>
#include <geekos/errno.h>
#include <geekos/projects.h>
#include <geekos/smp.h>
#include <geekos/bitset.h>
#include <geekos/vfs.h>


#include <libc/mmap.h>

/* ----------------------------------------------------------------------
 * Public data
 * ---------------------------------------------------------------------- */
pde_t *g_kernel_pde;

/* ----------------------------------------------------------------------
 * Private functions/data
 * ---------------------------------------------------------------------- */

#define SECTORS_PER_PAGE (PAGE_SIZE / SECTOR_SIZE)

/*
 * flag to indicate if debugging paging code
 */
int debugFaults = 1;
int numOfPagingPages = 0; //in the disk pagingfile.bin
void* BitmapPaging = 0; 
struct Paging_Device* pagingDevice;

#define Debug(args...) if (debugFaults) Print(args)


const pde_t *Kernel_Page_Dir(void) {
    TODO_P(PROJECT_VIRTUAL_MEMORY_A,
           "return kernel page directory and page tables");
    return g_kernel_pde;;
}


int myDebug = 0;
/*
 * Print diagnostic information for a page fault.
 */
int replace_val(){
    // if(CURRENT_THREAD->userContext->pages >= MAX_USER_PAGES)
    //     return 1;
    return -1;
}

static void Print_Fault_Info(uint_t address, faultcode_t faultCode) {
    extern uint_t g_freePageCount;

    // Print("Pid %d: ", CURRENT_THREAD->pid);
    Print("\n Page Fault received, at address %p (%d pages free)\n",
          (void *)address, g_freePageCount);
    if (faultCode.protectionViolation)
        Print("   Protection Violation, ");
    else
        Print("   Non-present page, ");
    if (faultCode.writeFault)
        Print("Write Fault, ");
    else
        Print("Read Fault, ");
    if (faultCode.userModeFault)
        Print("in User Mode\n");
    else
        Print("in Supervisor Mode\n");
}
//---------------------------------------------------------
int Alloc_User_Page(pde_t * pageDir, uint_t startAddress, uint_t sizeInMemory){
    
    Print("pagedir %d, startadd: %u, sizeInMemory: %d \n", (int)pageDir, startAddress, sizeInMemory);
    uint_t pagedir_index=startAddress>>22;
    uint_t page_index=(startAddress<<10)>>22;

    pde_t * pagedir_entry=pageDir+pagedir_index;
    pte_t * page_entry;

    // KASSERT(0);
    if(pagedir_entry->present){
        page_entry=(pte_t *)(pagedir_entry->pageTableBaseAddr<<12); 
    }
    else{
        // KASSERT(0);
        page_entry=(pte_t*) Alloc_Page();
        if(page_entry==NULL) {
            Print("can not allocate page in Alloc_User_Page/n");
            return -1;
        }
        memset(page_entry,0,PAGE_SIZE);
        *((uint_t*)pagedir_entry)=0;
        pagedir_entry->present=1;
        pagedir_entry->flags=VM_WRITE | VM_READ | VM_USER;
        // pagedir_entry->globalPage=0;
        pagedir_entry->pageTableBaseAddr=(ulong_t)page_entry >> 12;
    }
    // KASSERT(0);
    page_entry+=page_index;

    int num_pages;
    void * page_addr;
    num_pages=Round_Up_To_Page(startAddress-Round_Down_To_Page(startAddress)+sizeInMemory)/PAGE_SIZE;

    // Print("startAddress is %x,num_pages is %d\n",startAddress,num_pages);
    // KASSERT(0);

    int i;
    uint_t first_page_addr=0;
    for(i=0;i<num_pages;i++){
        // KASSERT(0);
        if(!page_entry->present){
          // KASSERT(0);
            // Print("PID: %d, cur pages %d \n", CURRENT_THREAD->pid, CURRENT_THREAD->userContext->pages);
            // CURRENT_THREAD->userContext->pages++;
            // KASSERT(0);            
            int x = -1;
            page_addr=Alloc_Pageable_Page(page_entry, Round_Down_To_Page(startAddress));            
            if(page_addr==NULL) {
                Print("can not allocate page in Alloc_User_Page\n");
                KASSERT(0);
                return -1;
            }
            // KASSERT(0);
            *((uint_t*)page_entry)=0;
            page_entry->present=1;
            page_entry->flags=VM_WRITE | VM_READ | VM_USER;
            // page_entry->flags= VM_USER;
            page_entry->globalPage = 0;
            page_entry->pageBaseAddr = (ulong_t)page_addr>>12;
            KASSERT(page_addr!= 0);
            if(i==0){
                first_page_addr = (uint_t) page_addr;
            } 
        }
        // KASSERT(0);
        page_entry++;
        startAddress+=PAGE_SIZE; 
    }
    return 0;
}
//---------------------------------------------------------


/*
 * Handler for page faults.
 * You should call the Install_Interrupt_Handler() function to
 * register this function as the handler for interrupt 14.
 */
static void Page_Fault_Handler(struct Interrupt_State *state) {
    // KASSERT(0);

   ulong_t address;
    faultcode_t faultCode;

    KASSERT(!Interrupts_Enabled());
 // Update_Clock();
    /* Get the address that caused the page fault */
    address = Get_Page_Fault_Address();
    // Debug("Page fault @%lx/n=========================", address);
    /* Get the fault code */
    faultCode = *((faultcode_t *) &(state->errorCode));
 
 struct User_Context* userContext = CURRENT_THREAD->userContext;
 if(faultCode.writeFault)
 { 
  //mydebug
  //Print("write Fault/n");
    // KASSERT(0);
  int res;
    //   Print("cur pages %d\n", CURRENT_THREAD->userContext->pages);
    // KASSERT(0);
  res=Alloc_User_Page(userContext->pageDir,Round_Down_To_Page(address),PAGE_SIZE);
  
  if(res==-1)
  {
   //Print("Alloc_User_Page error in Page_Fault_Handler/n");
   Exit(-1);
  }
  return ;
 }
 else
 { 
  //mydebug
  //Print("read fault/n");
  ulong_t page_dir_addr=address >> 22;
  ulong_t page_addr=(address << 10) >> 22;
  pde_t * page_dir_entry=(pde_t*)userContext->pageDir+page_dir_addr;
  pte_t * page_entry= NULL;
  if(page_dir_entry->present)
  {
   page_entry=(pte_t*)((page_dir_entry->pageTableBaseAddr) << 12);
   page_entry+=page_addr;
  }
  else
  { 
   Print_Fault_Info(address,faultCode);
   Exit(-1);
  }

  if(page_entry->kernelInfo!=KINFO_PAGE_ON_DISK)
  {
   Print_Fault_Info(address,faultCode);
   Exit(-1);
  }
  int pagefile_index = page_entry->pageBaseAddr;
  void * paddr=Alloc_Pageable_Page(page_entry,Round_Down_To_Page(address));
  if(paddr==NULL)
  {
   Print("no more page/n");
   Exit(-1);
  }

  *((uint_t*)page_entry)=0;
  page_entry->present=1;
  page_entry->flags=VM_WRITE | VM_READ | VM_USER;
  page_entry->globalPage = 0;
  page_entry->pageBaseAddr = (ulong_t)paddr>>12;
  Enable_Interrupts();
  KASSERT(0);
  Read_From_Paging_File(paddr,Round_Down_To_Page(address), pagefile_index);
  Disable_Interrupts();
  Free_Space_On_Paging_File(pagefile_index);
  return ;
 }

//     Print("In PF handler\n");
//     ulong_t address;
//     faultcode_t faultCode;

//     KASSERT(!Interrupts_Enabled());

//     /* Get the address that caused the page fault */
//     address = Get_Page_Fault_Address();

//     //Debug("Page fault @%lx\n", address);

//     if (address < 0xfec01000 && address > 0xf0000000) {
//         KASSERT0(0, "page fault address in APIC/IOAPIC range\n");
//     }
//     Print("In PF22 handler \n");
//     /* Get the fault code */
//     faultCode = *((faultcode_t *) & (state->errorCode));
//     Print_Fault_Info(address, faultCode);
//     Print("herere");
//     /* rest of your handling code here */
//     //TODO_P(PROJECT_VIRTUAL_MEMORY_B, "handle page faults");

//     //TODO_P(PROJECT_MMAP, "handle mmap'd page faults");

// //---------------------------------------------------------------
//     struct User_Context* userContext = CURRENT_THREAD->userContext;

//     if(faultCode.writeFault){ 
//         //Print("write Fault/n");
//         int res;
//         res=Alloc_User_Page(userContext->pageDir,Round_Down_To_Page(address),PAGE_SIZE);
//         if(res==-1){
//             //Print("Alloc_User_Page error in Page_Fault_Handler/n");
//             Exit(-1);
//         }
//         return ;
//     }
//     else{
//         ulong_t page_dir_addr= PAGE_DIRECTORY_INDEX(address);
//         ulong_t page_addr= PAGE_TABLE_INDEX(address);
//         pde_t * page_dir_entry=(pde_t*)userContext->pageDir+page_dir_addr;
//         pte_t * page_entry= NULL;
//         if(page_dir_entry->present){
//             page_entry=(pte_t*)((page_dir_entry->pageTableBaseAddr) << 12);
//             page_entry+=page_addr;
//         }
//         else{
//             Print_Fault_Info(address,faultCode);
//             Exit(-1);
//         }

//         if(page_entry->kernelInfo!=KINFO_PAGE_ON_DISK){
//             Print_Fault_Info(address,faultCode);
//             Exit(-1);
//         }
//         int pagefile_index = page_entry->pageBaseAddr;
//         void * paddr=Alloc_Pageable_Page(page_entry,Round_Down_To_Page(address));
//         if(paddr==NULL){
//             Print("no more page/n");
//             Exit(-1);
//         }

//         *((uint_t*)page_entry)=0;
//         page_entry->present=1;
//         page_entry->flags=VM_WRITE | VM_READ | VM_USER;
//         page_entry->globalPage = 0;
//         page_entry->pageBaseAddr = (ulong_t)paddr>>12;
//         Enable_Interrupts();
//         Read_From_Paging_File(paddr,Round_Down_To_Page(address), pagefile_index);//,page_entry);
//         Disable_Interrupts();
//         Free_Space_On_Paging_File(pagefile_index);
//         return ;
//     }
//     //---------------------------------------------------------------
//     error:
//     Print("Page fault @%lx\n", address);
//     Print("KASSSSSSSSSSSSSSSSSSSSSSSS\n");
//     Print("Unexpected Page Fault received\n");
//     Print_Fault_Info(address, faultCode);
//     Dump_Interrupt_State(state);
//     /* user faults just kill the process */
//     if (!faultCode.userModeFault)
//         KASSERT0(0, "unhandled kernel-mode page fault.");

//     /* For now, just kill the thread/process. */
//     Exit(-1);
}

void Idenity_Map_Page(pde_t * currentPageDir, unsigned int address, int flags) {
}

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */


/*
 * Initialize virtual memory by building page tables
 * for the kernel and physical memory.
 */
extern Interrupt_Handler* g_interruptTable;

void Init_VM(struct Boot_Info *bootInfo) {
   int num_dir_entries = (bootInfo->memSizeKB/4)/NUM_PAGE_TABLE_ENTRIES + 1;
    Print("%d", num_dir_entries);
    // KASSERT(0);
    g_kernel_pde =  Alloc_Page();
    if(g_kernel_pde == NULL)
        KASSERT(0);
    memset(g_kernel_pde, '\0', PAGE_SIZE);
    pte_t* cur_pte;
    pte_t *first_pte;
    pde_t* cur_pde;
    int i=0, j;
    uint_t mem;
    for(i=0; i<33; i++){
        g_kernel_pde[i].flags = VM_WRITE | VM_USER;
        g_kernel_pde[i].present = 1;
        // cur_pde->globalPage = 1;
        first_pte = Alloc_Page();
            if(first_pte == NULL)
                KASSERT(0); 
        memset(first_pte, '\0', PAGE_SIZE);
        g_kernel_pde[i].pageTableBaseAddr = ((uint_t)first_pte)>>12;
        int j;

        mem = i*NUM_PAGE_TABLE_ENTRIES*PAGE_SIZE;
        for(j=0; j<NUM_PAGE_TABLE_ENTRIES; j++){
            // cur_pte = &first_pte[j];
            // *cur_pte = 0;   
            // first_pte[j] = (struct pte_t){0};         
            if(i==0 && j==0){
                mem += PAGE_SIZE;
                continue;
            }

            first_pte[j].present = 1;
            // cur_pte->globalPage = 1;
            first_pte[j].flags = VM_WRITE | VM_USER;
            first_pte[j].pageBaseAddr = mem>>12;
            mem += PAGE_SIZE;
        }
    }

    // for(i = 960; i<=1019; i++){
    i = 1019;
    // cur_pde = &g_kernel_pde[i];
    g_kernel_pde[i].present = 1;
    // cur_pde->globalPage = 1;
    g_kernel_pde[i].flags = VM_WRITE | VM_USER;
    first_pte = Alloc_Page();
    if(first_pte == NULL)
        KASSERT(0);
    g_kernel_pde[i].pageTableBaseAddr = ((uint_t)first_pte)>>12;
    memset(first_pte, '\0', PAGE_SIZE);


    mem = i*NUM_PAGE_TABLE_ENTRIES*(long long)PAGE_SIZE;
    // Print("mem %x", mem);
    for(j=0; j<NUM_PAGE_TABLE_ENTRIES; j++){
        cur_pte = &first_pte[j];
        first_pte[j].present = 1;
        // cur_pte->globalPage = 1;
        first_pte[j].flags = VM_WRITE | VM_USER;
        first_pte[j].pageBaseAddr = mem>>12;
        mem += PAGE_SIZE;
    }
    // KASSERT(0);
    
    Enable_Paging(g_kernel_pde);
    Install_Interrupt_Handler(14, Page_Fault_Handler);
    Install_Interrupt_Handler(46, Page_Fault_Handler);

}


//     int  kernel_pde_entries;
//     int  whole_pages;
//     int  i,j;
//     uint_t mem_addr;
//     pte_t * cur_pte;

//     whole_pages=bootInfo->memSizeKB/4;
//     Print("whole pages are %d\n",whole_pages);
//     kernel_pde_entries=whole_pages/NUM_PAGE_TABLE_ENTRIES+1;//(whole_pages%NUM_PAGE_TABLE_ENTRIES==0 ? 0:1);
//     Print("kdep %d\n",kernel_pde_entries);
//     Print("the kernel_pde_entries is %d\n",kernel_pde_entries);
//     //KASSERT(0);

//     g_kernel_pde=(pde_t *)Alloc_Page();
//     Print("KERNALADD: %d\n", (int)g_kernel_pde);

//     memset(g_kernel_pde,'\0',PAGE_SIZE);

//     pde_t * cur_pde_entry;
//     cur_pde_entry=g_kernel_pde;
//     mem_addr=0;
//     for(i=0;i<kernel_pde_entries;i++){
//         cur_pde_entry->present=1;
//         cur_pde_entry->flags=VM_WRITE | VM_USER;
//        // cur_pde_entry->kernelInfo = KINFO_PAGE_ON_DISK;
//         // cur_pde_entry->accessed = 0;
//         // cur_pde_entry->reserved = 0;
//         cur_pde_entry->globalPage=1;
//         Print("I: %d\n", i);

//         cur_pte=(pte_t *)Alloc_Page();
//         KASSERT(cur_pte!=NULL);

//         memset(cur_pte,'\0',PAGE_SIZE);
//         cur_pde_entry->pageTableBaseAddr=(uint_t)cur_pte>>12;
//         mem_addr = i*NUM_PAGE_TABLE_ENTRIES*PAGE_SIZE;
//         int last_pagetable_num;
//         //last_pagetable_num=whole_pages%NUM_PAGE_TABLE_ENTRIES;
//         // if(last_pagetable_num==0 || i!=(kernel_pde_entries-1)){
//         //   last_pagetable_num=NUM_PAGE_TABLE_ENTRIES;
//         // }
//           last_pagetable_num=NUM_PAGE_TABLE_ENTRIES;

//         // int temp = 0;
//         // if (i == 0){
//         //     temp = 1;
//         //     cur_pte++;
//         //     mem_addr+=PAGE_SIZE;
//         // }
//         for(j=0;j<last_pagetable_num;j++){
//             // if(i==0 && j==0) {
//             //     cur_pte++;
//             //     continue;
//             // }
//             cur_pte->present=1;
//             cur_pte->flags=VM_WRITE|VM_USER;
//             cur_pte->globalPage=1;  
//             cur_pte->pageBaseAddr=mem_addr>>12;
//             //cur_pte->kernelInfo = KINFO_PAGE_ON_DISK;
//             cur_pte++;
//             mem_addr+=PAGE_SIZE;
//         }
//         cur_pde_entry++;
//         Print("%d curpde %d\n", i, (int)cur_pde_entry);
//     }
//     // KASSERT(0);
//     //APIC HOLE
//     cur_pde_entry = &g_kernel_pde[1019];
//     // cur_pde_entry += 1019;
//     cur_pde_entry->present=1;
//     cur_pde_entry->flags=VM_WRITE | VM_USER;
//     cur_pte=(pte_t *)Alloc_Page();

//     KASSERT(cur_pte!=NULL);

//     mem_addr = 1019*NUM_PAGE_TABLE_ENTRIES*(long long)(PAGE_SIZE);
//     memset(cur_pte,'\0',PAGE_SIZE);
//     cur_pde_entry->pageTableBaseAddr=(uint_t)cur_pte>>12;

//     for(j = 0;j < NUM_PAGE_TABLE_ENTRIES;j++){
//         cur_pte->present=1;
//         cur_pte->flags=VM_WRITE|VM_USER;
//         cur_pte->pageBaseAddr=mem_addr>>12;
//         cur_pte++;
//         mem_addr+=PAGE_SIZE;
//     }


//     //int *a = 4096;
//     Print("CUr thread%d\n", (int)CURRENT_THREAD);

//     Print("Converted: %d\n", ((int)(((pte_t*)(g_kernel_pde->pageTableBaseAddr<<12)+1)->pageBaseAddr<<12))+1);

//     Enable_Paging(g_kernel_pde);
//     Print("%d", (int)*g_interruptTable);
//     //KASSERT(0);    
//     Print("dddddddddddddddddddddddddddd");
//     myDebug = 1;
//     Install_Interrupt_Handler(14,Page_Fault_Handler);
//         if(myDebug){
//         Print("heeeeeeeeeeeeeeeee");
//         // KASSERT(0);
//     }
// }

void Init_Secondary_VM() {
    TODO_P(PROJECT_VIRTUAL_MEMORY_A, "enable paging on secondary cores");
}

/**
 * Initialize paging file data structures.
 * All filesystems should be mounted before this function
 * is called, to ensure that the paging file is available.
 */
void Init_Paging(void) {
    pagingDevice = Get_Paging_Device();
    if(pagingDevice==NULL){
        Print("can not find pagefile/n");
        KASSERT(0);
        return;
    }
    numOfPagingPages = pagingDevice->numSectors/SECTORS_PER_PAGE;
    BitmapPaging = Create_Bit_Set(numOfPagingPages);

    Print("%d",(int)numOfPagingPages);
    //KASSERT(0);
    //TODO_P(PROJECT_VIRTUAL_MEMORY_B,
      //     "Initialize paging file data structures");
}

/**
 * Find a free bit of disk on the paging file for this page.
 * Interrupts must be disabled.
 * @return index of free page sized chunk of disk space in
 *   the paging file, or -1 if the paging file is full
 */
int Find_Space_On_Paging_File(void) {
    KASSERT(!Interrupts_Enabled()); 
    return Find_First_Free_Bit(BitmapPaging, numOfPagingPages);     
    //TODO_P(PROJECT_VIRTUAL_MEMORY_B, "Find free page in paging file");
    //return EUNSUPPORTED;
}

/**
 * Free a page-sized chunk of disk space in the paging file.
 * Interrupts must be disabled.
 * @param pagefileIndex index of the chunk of disk space
 */
void Free_Space_On_Paging_File(int pagefileIndex) {
        KASSERT(0);

    KASSERT(!Interrupts_Enabled());
     KASSERT(0 <= pagefileIndex && pagefileIndex < numOfPagingPages);
     Clear_Bit(BitmapPaging, pagefileIndex);    
    //TODO_P(PROJECT_VIRTUAL_MEMORY_B, "Free page in paging file");
}

/**
 * Write the contents of given page to the indicated block
 * of space in the paging file.
 * @param paddr a pointer to the physical memory of the page
 * @param vaddr virtual address where page is mapped in user memory
 * @param pagefileIndex the index of the page sized chunk of space
 *   in the paging file
 */
void Write_To_Paging_File(void *paddr, ulong_t vaddr, int pagefileIndex) {
    struct Page *page = Get_Page((ulong_t) paddr);
    KASSERT(!(page->flags & PAGE_PAGEABLE)); /* Page must be locked! */
    KASSERT((page->flags & PAGE_LOCKED));
 //Debug("PageFileIndex: 0 <= %d < %d/n", pagefileIndex, bitmapSize);
    if(0 <= pagefileIndex && pagefileIndex < numOfPagingPages){
        int i;
        for(i = 0;i < SECTORS_PER_PAGE; i++){
            Block_Write(pagingDevice->dev, pagefileIndex*SECTORS_PER_PAGE + i + (pagingDevice->startSector),paddr+i*SECTOR_SIZE);      
        }
        Set_Bit(BitmapPaging, pagefileIndex); 
    }
    else {
        Print("In func Write_To_Paging_File: pagefileIndex out of range!");
        KASSERT(0);
        Exit(-1);
    } 
    KASSERT(0); 
    //struct Page *page = Get_Page((ulong_t) paddr);
    //KASSERT(!(page->flags & PAGE_PAGEABLE));    /* Page must be locked! */
    // TODO_P(PROJECT_VIRTUAL_MEMORY_B, "Write page data to paging file");
}

/**
 * Read the contents of the indicated block
 * of space in the paging file into the given page.
 * @param paddr a pointer to the physical memory of the page
 * @param vaddr virtual address where page will be re-mapped in
 *   user memory
 * @param pagefileIndex the index of the page sized chunk of space
 *   in the paging file
 */
void Read_From_Paging_File(void *paddr, ulong_t vaddr, int pagefileIndex) {
        KASSERT(0);

    struct Page *page = Get_Page((ulong_t) paddr);
    KASSERT(!(page->flags & PAGE_PAGEABLE));    /* Page must be locked! */

    if(0<=pagefileIndex && pagefileIndex<numOfPagingPages){
        int i;
        for(i=0;i<SECTORS_PER_PAGE;i++){
            Block_Read(pagingDevice->dev, pagefileIndex*SECTORS_PER_PAGE + i + (pagingDevice->startSector),paddr+i*SECTOR_SIZE);      
        }
    }
    else {
        Print("In func Write_To_Paging_File: pagefileIndex out of range!");
        KASSERT(0);
        Exit(-1);
    }     

    //TODO_P(PROJECT_VIRTUAL_MEMORY_B, "Read page data from paging file");
}


void *Mmap_Impl(void *ptr, unsigned int length, int prot, int flags, int fd) {
    TODO_P(PROJECT_MMAP, "Mmap setup mapping");
    return NULL;
}

bool Is_Mmaped_Page(struct User_Context * context, ulong_t vaddr) {
    TODO_P(PROJECT_MMAP,
           "is this passed vaddr an mmap'd page in the passed user context");
    return false;
}

void Write_Out_Mmaped_Page(struct User_Context *context, ulong_t vaddr) {
    TODO_P(PROJECT_MMAP, "Mmap write back dirty mmap'd page");
}

int Munmap_Impl(ulong_t ptr) {
    TODO_P(PROJECT_MMAP, "unmapp the pages");
}