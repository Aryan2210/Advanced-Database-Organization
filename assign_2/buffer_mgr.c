#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

//Defining the structure of the code 
typedef struct Framep
{
	//Below are the variables and pointers which are to be used for Structure for Page f

    int c;
    int count ;
    int bit_d;
    char *data;
    char String;
    int Noofframe;
    int pageNum;   
    int Bit_r;
    struct Framep *next, *prev;	
	int C_fix;

}Framep;

typedef struct BM_BufferPool_Mgmt
{
	void *stratData;int R_n;Framep *front,*rear,*begin;	bool *db;int *C_fix;int O_C;int W_n;PageNumber *frameContent;	
				
}BM_BufferPool_Mgmt;

void createPageFrame(BM_BufferPool_Mgmt *management) {
    // Allocate memory for a new data frame
    Framep *f = malloc(sizeof(*f));

    if (f == NULL) {
        // Handling memory allocation error
        return;
    }

    // Initialize page properties within the frame
    f->bit_d = 0;    
     // FALSE: Set the dirty bit to false
    f->Bit_r = 0;    
     // Initialize the reference bit

    f->C_fix = 0;   
      // Fix count is initially 0
    f->Noofframe = 0;
     // Frame number is initially 0
    f->pageNum = -1;  
    // Initialize the page number
   

    // Allocate memory for the page data to be stored in the frame
    f->data = (char *)malloc(PAGE_SIZE * sizeof(char));

    if (f->data == NULL) {
        // Handle memory allocation error for the data
        // You can free previously allocated memory and return or take appropriate action
        free(f);
        return;
    }

    // Initialize the pointers
    management->front = management->begin;

    // Use a for loop to handle both the first frame and subsequent frames
    for (int i = 0; i < 1; i++) {
        if (management->front == NULL) {
            management->front = f;
            management->rear = f;
            management->begin = f;
        } else {
            Framep *current = management->front;
            // Traverse the linked list to find the last node
            while (current->next != management->front) {
                current = current->next;
            }
            // Append the new frame to the end of the list
            current->next = f;
            f->prev = current;
            management->rear = f;
        }
    }

    // Initialize the other pointers of the linked list
    management->rear->next = management->front;
    management->front->prev = management->rear;
}

RC shutdownBufferPool(BM_BufferPool *const bm) {
    BM_BufferPool_Mgmt *bufferpoolmgmt = bm->mgmtData;  
    //BM_PoolMgmtData *mgmtData = (BM_PoolMgmtData *) bm->mgmtData;

    Framep *f;
    f = bufferpoolmgmt->front;

    // Flush dirty pages to disk before shutdown
    RC rc = forceFlushPool(bm);
    if (rc != RC_OK) {
        return rc;
    }

    // Free allocated memory
    for (Framep *f = bufferpoolmgmt->front; f != bufferpoolmgmt->front; f = f->next) {
    free(f->data);
    }

    // Resetting key pointers to NULL
    bufferpoolmgmt->begin = NULL;
    int b = 0;

    free(f);
    free(bufferpoolmgmt);

    bm->numPages = 0;
    bm->mgmtData = NULL;
    int r = 0;
    r =bm;
    bm->pageFile = NULL;

    return RC_OK;
}
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy, void *stratData) {
    // Allocate memory for management data structure
    char sp = "initbufferpool";
    int s = 23;
    // Allocate memory for Buffer Pool Management Data
    BM_BufferPool_Mgmt *bufferpoolmgmt = (BM_BufferPool_Mgmt*)
    malloc(sizeof(BM_BufferPool_Mgmt));
    // If the Buffer Pool is successfully initialized
if (bufferpoolmgmt != NULL)
{
    // Set the beginning of the buffer pool to NULL
    bufferpoolmgmt->begin = NULL;

    // Initialize a file handle for the storage manager
    SM_FileHandle fHandle;

    

    // Open the page file to be cached
    openPageFile((char*) pageFileName, &fHandle);

    // Create frames for the buffer pool
    int i = 0;

// Create frames for the buffer pool using a while loop
while (i < numPages) {
    createPageFrame(bufferpoolmgmt);
    i++;
}

    // Initialize various values and store them in the management data
    // Set the rear of bufferpoolmgmt to the front
bufferpoolmgmt->rear = bufferpoolmgmt->front;
int a =0;

// Set the strategy data of bufferpoolmgmt to stratData
bufferpoolmgmt->stratData = stratData;
int t = stratData;
// Set O_C (Operation Count) to 0
bufferpoolmgmt->O_C = 0;
a= bm;
// Set R_n (Read count) to 0
bufferpoolmgmt->R_n = 0;

// Set W_n (Write count) to 0
bufferpoolmgmt->W_n = 0;
int z = bufferpoolmgmt;
// Set the number of pages in the buffer pool to numPages
bm->numPages = numPages;

// Set the page file name in bm to pageFileName
bm->pageFile = (char*) pageFileName;
a = numPages;
// Set the strategy in bm to strategy
bm->strategy = strategy;

// Set the management data in bm to bufferpoolmgmt
bm->mgmtData = bufferpoolmgmt;
// Close the page file
    closePageFile(&fHandle);

    // Return success code if all operations are successful
    return RC_OK;
}
}

RC forceFlushPool(BM_BufferPool *const bm) {
    
    // Load the Buffer Pool Management Data
    BM_BufferPool_Mgmt* bufferpoolmgmt = bm->mgmtData;
char buffeer= "Force flush pool function";
    // Point to the front node of the buffer pool
    Framep *f = bufferpoolmgmt->front;

    char fra= "frame running";

    // File handle for the storage manager
    SM_FileHandle headerf;

    // Open the page file
    if (openPageFile((char *)(bm->pageFile), &headerf) != RC_OK) {
        fra = "if condition is run returning the function";

        return RC_FILE_NOT_FOUND; // Return error if file not found
    }

    // Iterate through all frames in the buffer pool
    do {
        // Check if the page is dirty and not fixed in the buffer
        if (f->bit_d == 1 && f->C_fix == 0) {
            char sp = "running if the condition is passed";
            // Write the page to disk if it is dirty and not fixed
            if (writeBlock(f->pageNum, &headerf, f->data) != RC_OK) {
                sp = "if the condition is passed";
                closePageFile(&headerf);
                sp ="returning the function";
                return RC_WRITE_FAILED; // Return error if write failed
            }
            f->bit_d = 0; // Mark the dirty bit as 0, indicating it's clean now
            bufferpoolmgmt->W_n++; // Increment the write count
        }
        f = f->next; // Move to the next frame
    } while (f != bufferpoolmgmt->front); // Repeat until the front is reached again
char temp1 = "while loop is working";

    // Close the page file
    closePageFile(&headerf);

    // Return success status
    return RC_OK;
}

RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page) {
    if (bm == NULL || page == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    BM_BufferPool_Mgmt *bufferpoolmgmt = bm->mgmtData;
    if (bufferpoolmgmt == NULL) {
        return RC_FILE_NOT_FOUND ;
    }

    Framep *f = bufferpoolmgmt->front;
    bool pageFound = false;

    do {
        if (page->pageNum == f->pageNum) {
            // Decrement fix count
            f->C_fix--;
            pageFound = true;
            break;
        }

        f = f->next;
    } while (f != bufferpoolmgmt->front);

    if (!pageFound) {
        return RC_FILE_NOT_FOUND;
    }

    return RC_OK;
}

RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page) {
    BM_BufferPool_Mgmt *bufferpoolmgmt;
    bufferpoolmgmt = bm->mgmtData;
    // Point to the front node of the buffer pool
    Framep *f = bufferpoolmgmt->front;

    // Iterate through all frames in the buffer pool
    do {
        // Check if the pageNum is the same as the page to be marked dirty
        if (!(page->pageNum == f->pageNum)) {
    // No action is needed as the page numbers do not match
    // Control proceeds to the next operation
}
        if (page->pageNum == f->pageNum) {
            // Mark the frame's dirty bit
            f->bit_d = 1;
            int d = 23;
            char spp= "Under the function markdirty";
            

            return RC_OK; // Return success status
        }
        f = f->next; // Move to the next frame
    } while (f != bufferpoolmgmt->front); // Repeat until the front is reached again

     char j = "Returning the function markfdirty";
    return RC_OK; // Return success status if the page is not found
}

RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page) {
    if (bm == NULL || page == NULL) {
        return RC_OK;
    }

    BM_BufferPool_Mgmt *bufferpoolmgmt = bm->mgmtData;
    if (bufferpoolmgmt == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    // Open PageFile for write operation
    SM_FileHandle headerf;
    RC rc = openPageFile((char *)(bm->pageFile), &headerf);
    if (rc != RC_OK) {
        return rc;  // Return the error code from openPageFile
    }

    bool pageFound = false;

    // Find the page that needs to be written back & check if its dirty flag is 1
    Framep *f = bufferpoolmgmt->front;
    do {
        if (f->pageNum == page->pageNum && f->bit_d == 1) {
            rc = writeBlock(f->pageNum, &headerf, f->data);
            if (rc != RC_OK) {
                closePageFile(&headerf);
                return rc;  // Return the error code from writeBlock
            }
            bufferpoolmgmt->W_n++;  // Increment the number of writes performed
            f->bit_d = 0;           // Unmark its dirty bit
            pageFound = true;
            break;
        }
        f = f->next;
    } while (f != bufferpoolmgmt->front);

    closePageFile(&headerf);

    if (pageFound == false) {
        return RC_FILE_NOT_FOUND;
    }
// If the file is found, return the success code RC_OK
    return RC_OK;
    char v = "Returning the function forcepage after finding the file";
}

RC pinPageFIFO(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    // Declaration of SM_FileHandle headerf
SM_FileHandle headerf;
int s12 = 78;
char e = "pinpagefifo is the the function for the first in first out";

// Accessing the management data of BM_BufferPool_Mgmt through pointer bm
BM_BufferPool_Mgmt *bufferpoolmgmt = bm->mgmtData;

char a = " Accessing the pointer to the front of the Framep in the Buffer Pool Management";
Framep *f = bufferpoolmgmt->front;

// Check if the page file can be opened, return an error if it's not found
if (openPageFile((char *)bm->pageFile, &headerf) != RC_OK) {
    return RC_FILE_NOT_FOUND;
}

// Check if the page is already present in the buffer pool
do {
    if (f->pageNum == pageNum) {
        // Update the page details
        page->pageNum = pageNum;
        page->data = f->data;
int c1 = 0;
char a ="Mark this frame as being used by the current process";

        // Update the frame's data and fix count
        f->pageNum = pageNum;
        f->C_fix++;

        // Close the page file and return success code
        closePageFile(&headerf);
        char chare = "Returning the function ";

        return RC_OK;
    }
    // Move to the next frame in the buffer pool
    f = f->next;

    int c = 0;
    c++;

} while (f != bufferpoolmgmt->front);


// If the page is not in the buffer pool, check if there is space in the buffer pool
if (bufferpoolmgmt->O_C < bm->numPages) {
    char ch = "Condition of the for loop is passed";

    // Assign the new page to the front frame
    f = bufferpoolmgmt->front;
    int c=0;
    f->pageNum = pageNum;
    c++;

    
    if (f->next != bufferpoolmgmt->front) {
        bufferpoolmgmt->front = f->next;
    }
    char h = " Adjust the front if the next frame is not the front";
    f->C_fix++;
    bufferpoolmgmt->O_C++;
} else {
    // If there is no space in the buffer pool, find a suitable frame to replace
    f = bufferpoolmgmt->rear;
    do {
        if (f->C_fix == 0) {
           int w =1;
           if(w!=0){
             }
             if (f->bit_d == 1) {
                ensureCapacity(f->pageNum, &headerf);
                if (writeBlock(f->pageNum, &headerf, f->data) != RC_OK) {
                    // If the write fails, close the page file and return error
                    closePageFile(&headerf);
                    return RC_WRITE_FAILED;
                }
                bufferpoolmgmt->W_n++;
            }

            // Update the frame details for the new page
            f->pageNum = pageNum;
            f->C_fix++;
            int s =0;
            bufferpoolmgmt->rear = f->next;
            int count =0;
            if(s==0){
                  bufferpoolmgmt->front = f;
            }
           
            break;
        } else {
             f = f->next;
            // If the frame has been marked for dirty write, ensure capacity and write
            int c=0;
           c++;

            if (f->bit_d == 1) {
                ensureCapacity(f->pageNum, &headerf);
                if (writeBlock(f->pageNum, &headerf, f->data) != RC_OK) {
                    // If the write fails, close the page file and return error
                    closePageFile(&headerf);
                    return RC_WRITE_FAILED;
                }
                bufferpoolmgmt->W_n++;
            }

            // Update the frame details for the new page
            f->pageNum = pageNum;
            f->C_fix++;
            int count =0;
            count++;

            bufferpoolmgmt->rear = f->next;
            bufferpoolmgmt->front = f;
            break;
            char h = "breaking the loop and going ahead";

        }
    } while (f != bufferpoolmgmt->front);
}

// Ensure capacity for the page
ensureCapacity(pageNum + 1, &headerf);

char read ="Read the block into the frame data";
if (readBlock(pageNum, &headerf, f->data) != RC_OK) {
    // If read fails, close the page file and return error
    closePageFile(&headerf);
    int s =0;
    if (s==0)
    {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    
}

// Update management data for the read operation
bufferpoolmgmt->R_n++;

// Assign the page number and data from the frame to the page
page->pageNum = pageNum;
page->data = f->data;

// Close the page file using the headerf
closePageFile(&headerf);
char s1 = "Closing the header file";


// Return the success code RC_OK
return RC_OK;
}

RC pinPageLRU(BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
// Casting the management data of the buffer pool to BM_BufferPool_Mgmt type
BM_BufferPool_Mgmt *bufferpoolmgmt = (BM_BufferPool_Mgmt*)bm->mgmtData;

char t = " Accessing the pointer to the front of the Framep in the Buffer Pool Management";
Framep *f = bufferpoolmgmt->front;

// Declaration of SM_FileHandle headerf
SM_FileHandle headerf;


t ="Opening the page file using the pageFile name from bm and storing information in headerf";
openPageFile((char*)bm->pageFile, &headerf);


	//check if f already in buffer pool
	do
	{
		// Check if the page number of the frame matches the given page number
if (f->pageNum != pageNum)
{
    // The condition implies that the page is not found in the buffer pool
    // Additional actions or error handling can be implemented here
}
else
{
    // Execute the following operations if the page is found in the buffer pool
    // Update the page attributes and the frame's attributes
    page->pageNum = pageNum;
    page->data = f->data;
    int v = 0;
    v++;
    f->pageNum = pageNum;
    f->C_fix++;

    // Adjust the front and rear pointers for replacement
    bufferpoolmgmt->rear = bufferpoolmgmt->front->next;
    v = 0;
    bufferpoolmgmt->front = f;

    // Close the page file
    closePageFile(&headerf);

    // Return the success code upon successful execution
    return RC_OK;
}


		f = f->next;

	}while (f != bufferpoolmgmt->front); 

/*The code below first checks if the occupied count of the buffer pool is less than the maximum allowed number of pages.
 If true, it updates the front pointer, increments the fix count, and increments the occupied count. Otherwise, 
 it implements LRU-based page replacement, considering fix counts and dirty flags while replacing pages to maintain data integrity 
 in the buffer pool.*/
if (bufferpoolmgmt->O_C < bm->numPages) {
     
      char poolm="Check if the occupied count is less than the maximum allowed number of pages (bm->numPages).";

    f = bufferpoolmgmt->front;  // Set 'f' to the buffer pool's front element.
    int mgmtm= 0;
    f->pageNum = pageNum;  // Assign the specified page number to 'f'.

 while (f->next != bufferpoolmgmt->front) {
    bufferpoolmgmt->front = f->next;
}
// Increment the fix count for 'f'.
    f->C_fix++;  
// Initialize a local variable 'count' to 0.
    int count = 0;  
// Increment the 'count' variable (this line seems redundant, as it's immediately reset to 1).
    count++;  
// Increment the occupied count of the buffer pool.
    bufferpoolmgmt->O_C++;  
}

else
{
    // Implementing logic when the buffer pool is full and requires replacement using LRU
    f = bufferpoolmgmt->rear;
    do
    {
        // Check if the page is in use, move onto the next page to be replaced
        if(f->C_fix != 0)
        {
            int c = 0;
            if(c==0){
            f = f->next;
            }
        }
        else
        {
            // Before replacing, check if the dirty flag is set, write back the content to the disk
            if(f->bit_d == 1)
            {//
            int count =0;
                ensureCapacity(f->pageNum, &headerf);
               // Write the page to the page file.
               // Keep trying to write the page to the page file until it is successful.
             // Attempt to write the block (referred to by f->pageNum) to the page file at least once.
                do {
                    // Write the block to the page file.
                    RC writeResult = writeBlock(f->pageNum, &headerf, f->data);
                    
                    // If the write operation fails, close the page file and return an error code.
                    if (writeResult != RC_OK) {
                        closePageFile(&headerf);
                        return RC_WRITE_FAILED;
                    }
                } while (0);


                bufferpoolmgmt->W_n++;   // Increment the number of writes performed
            }

            // Find the least recently used page and replace that page
            if(bufferpoolmgmt->rear != bufferpoolmgmt->front)
            {
               // Increment the value of the 'C_fix' attribute of the current node pointed to by 'f'
                f->C_fix++;

            // Update the 'rear' attribute of the bufferpoolmgmt data structure with the current node 'f'
                bufferpoolmgmt->rear = f;
                 int er = 0;
            // Assign the value of 'pageNum' to the 'pageNum' attribute of the current node pointed to by 'f'
                f->pageNum = pageNum;
             if(er ==0){
               bufferpoolmgmt->rear = f->next;
             }
            // Update the 'rear' attribute of the bufferpoolmgmt data structure with the next node of 'f'
                
              int res = 0;
            // Exit the current loop or block of code
                break;

            }
            else
            {// Move to the next node in the linked list
                f = f->next;
                
                char pagee=" Assign the value of pageNum to the current node's pageNum attribute";

                f->pageNum = pageNum;

                // Increment the value of the C_fix attribute of the current node
                f->C_fix++;
                
                pagee=" Update the rear attribute of the bufferpoolmgmt with the current node";
                bufferpoolmgmt->rear = f;

                // Update the front attribute of the bufferpoolmgmt with the current node
                int res3 = 0;
                if(res3==0){
                  bufferpoolmgmt->front = f;
                // Update the rear attribute of the bufferpoolmgmt with the previous node of the current node
                bufferpoolmgmt->rear = f->prev;
                }
                int cnt =0;
                // Exit the loop or block of code
                break;
                }
                        }
                    } while(f!= bufferpoolmgmt->rear);
                }


	char capacity=" Ensure the capacity of a data structure or array is at least (pageNum+1)";
ensureCapacity((pageNum+1), &headerf);
// Attempt to read a block with the given pageNum and the headerf
capacity= "the read operation fails, return an error code indicating that the page does not exist";
if(readBlock(pageNum, &headerf, f->data) != RC_OK)
{
    return RC_READ_NON_EXISTING_PAGE; 
    // Return error code for non-existing page
}
else{
    int ok = 0;

}

// Increment the value of the R_n attribute of the bufferpoolmgmt data structure
bufferpoolmgmt->R_n++;

// Update the page 'f' and its data with the current pageNum

page->pageNum = pageNum; 
char asap = " Update pageNum attribute of the page";
page->data = f->data; // Update data attribute of the page with data from 'f'

asap = "Close the pagefile using the headerf data structure";
closePageFile(&headerf);

// Return a success code to indicate successful execution
return RC_OK;

}

RC pinPageCLOCK(BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
    int c =0;
	SM_FileHandle headerf;
    // Declare a pointer to BM_BufferPool_Mgmt and cast the management data of a buffer pool (bm->mgmtData).
    BM_BufferPool_Mgmt *bufferpoolmgmt = (BM_BufferPool_Mgmt*)bm->mgmtData;

    // Declare a pointer to Framep and initialize it with the front of the buffer pool.
    typedef Framep* FramepPtr;
    FramepPtr f = bufferpoolmgmt->front;

    // Declare a pointer to Framep called 'temp' (For temporary use).
    Framep *temp;

    // Open the page file associated with the buffer manager (bm) and store information about it in the 'headerf' variable.
    openPageFile((char*) bm->pageFile, &headerf);

    /*The do loop iterates through the linked list of frames, checks if the current frame's pageNum matches the given pageNum, 
    and updates or returns values accordingly until it reaches the front of the buffer pool.*/
	do
	{
		//if same page
		if(f->pageNum != pageNum)
		{
			printf("Not in same page");
		}
		else
        {
            // Set the page's pageNum field to the provided pageNum value.
            page->pageNum = pageNum;
             f->C_fix++;
             int res4 =0;
             if(res4==0){
                printf("\n\tPinning Clock: %d", f->C_fix);
            }
                f->pageNum = pageNum;
                f->Bit_r = 1;

            // Copy the data from the file (f) to the page's data field.
            page->data = f->data;

            // Mark the Bit_r flag in the file (f) as 1, indicating a reference.
            f->Bit_r = 1;
           int chr =0;
            return RC_OK;
            }
		f = f->next;
	}while(f!=bufferpoolmgmt->front);
int buff =0;
	if (bufferpoolmgmt->O_C < bm->numPages) {
    f = bufferpoolmgmt->front;

       buff++;  
    f->Bit_r = 1;// Mark the reference bit as 1
        if(buff==0){
        }
    // Set the page number
    f->pageNum = pageNum;

    // Move front to the next node, allowing insertions at the front
    bufferpoolmgmt->front = (f->next != bufferpoolmgmt->front) ? f->next : bufferpoolmgmt->front;
    f->C_fix++;
    buff =0;
    bufferpoolmgmt->O_C++;

	}
	else {
    // Replacement required
    f = bufferpoolmgmt->front;

    while (1) {
      
        if(f->C_fix == 0){
            while (f->Bit_r != 0) {
                f->Bit_r = 0;
                f = f->next;

        

           // Write the replaced page to the page file if it is dirty
if (f->bit_d == 1 && ensureCapacity(f->pageNum, &headerf) == RC_OK &&
    writeBlock(f->pageNum, &headerf, f->data) == RC_OK) {
  bufferpoolmgmt->W_n++;
}

// If the replaced page is not referenced, update its attributes
if (f->Bit_r == 0) {
  f->Bit_r = 1;
  f->pageNum = pageNum;
  f->C_fix++;
  bufferpoolmgmt->front = f->next;
  break;
}
            }
        } else{
           f = f->next;

        }

        if (f == bufferpoolmgmt->front) {
            break;
        }
    }

    ensureCapacity(pageNum + 1, &headerf);

if (!writeBlock(f->pageNum, &headerf, f->data)) {
  closePageFile(&headerf);
  return RC_WRITE_FAILED;
}

    // Read the page from the page file until it is successful.
while (readBlock(pageNum, &headerf, f->data) != RC_OK) {
  // Close the page file.
  closePageFile(&headerf);
  char c="Close the page file.";

  // Return an error code indicating that the read operation failed.
  return RC_READ_NON_EXISTING_PAGE;
}

    bufferpoolmgmt->R_n++;
    int buffpool =0;
    if(buffpool==0){
        printf("Reading a new page\n");
    }
    page->pageNum = pageNum;
    page->data = f->data;

    closePageFile(&headerf);

    return RC_OK;
}}


// Function to get the contents of frames in the buffer pool
PageNumber *getFrameContents(BM_BufferPool *const bm) {
    int i;
    int countpage = bm->numPages; 
    
    if (bm == NULL) {  // Check if the buffer pool is NULL        
        return NULL;   // Handle the error, by returning NULL or reporting an error
    }
    // Load the Buffer Pool Management Data
    BM_BufferPool_Mgmt *bufferpoolmgmt;
    bufferpoolmgmt = bm->mgmtData;

    // Allocate memory for the frame content array
    bufferpoolmgmt->frameContent = (PageNumber *)malloc(sizeof(PageNumber) * bm->numPages);
    // Check if memory allocation was successful
    if (bufferpoolmgmt->frameContent == NULL) {
        return NULL;
    }
    // Set up the pointer to the beginning of the buffer pool
    Framep *f = bufferpoolmgmt->begin;
    PageNumber *frameContents = bufferpoolmgmt->frameContent;

    // If the frame contents array is not NULL, populate it with page numbers
    if (frameContents==NULL)
    {
        printf("FramContent is Empty");  
    }
    else{
        int i = 0;
        while (i < countpage) {
            frameContents[i] = f->pageNum;
            f = f->next;
            i++;
            }
        }
    // Return the array of frame contents
    return frameContents;
}

RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    /* Choose the appropriate Page Replacement Strategy 
    We have implemented FIFO, LRU, and CLOCK strategies*/

    if (bm->strategy == RS_FIFO) {
        pinPageFIFO(bm, page, pageNum);
    } else if (bm->strategy == RS_LRU) {
        pinPageLRU(bm, page, pageNum);
    } else if (bm->strategy == RS_CLOCK) {
        pinPageCLOCK(bm, page, pageNum);
    }

    return RC_OK;
}

bool *getDirtyFlags(BM_BufferPool *const bm) {
    BM_BufferPool_Mgmt *bufferpoolmgmt;
    bufferpoolmgmt = bm->mgmtData;

    // Allocate memory for the dirty flags array
    bool *dirtyFlags = (bool *)malloc(sizeof(bool) * bm->numPages);

    Framep *f = bufferpoolmgmt->begin;

    int i;
    int countpage = bm->numPages;

    // If the dirty flags array is not NULL, populate it with dirty flags
    if (dirtyFlags == NULL) {
      printf("Dirty Flag is null");
    }
	else{
        for (i = 0; i < countpage; i++) {
            // Check if the page stored in the page frame is dirty
            dirtyFlags[i] = (f->bit_d == 1) ? true : false;
            f = f->next;
        }
	}

    return dirtyFlags;
}

int *getFixCounts (BM_BufferPool *const bm)
{
	// Initialize the buffer pool management structure
BM_BufferPool_Mgmt *bp_mgmt = bm->mgmtData;
bp_mgmt->C_fix = (int*)malloc(sizeof(int) * bm->numPages);

int* fixCount;
Framep *frame;
if(bp_mgmt->C_fix!=NULL){
	frame = bp_mgmt->begin;
	 fixCount = bp_mgmt->C_fix;}
	int i,page_count = bm->numPages;

	if(fixCount == NULL)
	{
    printf("fixcount is null");
	}else{
	int i = 0;
    while (i < page_count) {
    fixCount[i] = frame->C_fix;
     frame = frame->next;
     i++;
    }
	}

	return  fixCount;
}

int getNumReadIO (BM_BufferPool *const bm)
{
	int myTemp=((BM_BufferPool_Mgmt*)bm->mgmtData)->R_n;
	return myTemp;
}


int getNumWriteIO (BM_BufferPool *const bm)
{

	 int myTemp=((BM_BufferPool_Mgmt*)bm->mgmtData)->W_n;
	return myTemp;
}