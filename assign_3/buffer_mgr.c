#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

//Defining the structure of the code 
typedef struct Page
{
	SM_PageHandle data; 
	PageNumber pageNum; 
	int bit_d; 
	int fCount; 
	int hitNum;   	
	int refNum;   
} Framep;

// "buffer_size" represents the size of the buffer pool i.e. maximum number of page frames that can be kept into the buffer pool
int buffer_size = 0;

// "hit" a general count which is incremented whenever a page frame is added into the buffer pool used by LRU.
int hit = 0;

// "clock_pointer" is used by CLOCK algorithm to point to the last added page in the buffer pool.
int clock_pointer = 0;

// "rear_Index" basically stores the count of number of pages read from the disk also used by FIFO function.
int rear_Index = 0;

// "wcount" counts the number of I/O write to the disk i.e. number of pages writen to the disk
int wcount = 0;

extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
    Framep *Fpage = (Framep *)bm->mgmtData;

    // Write all dirty pages (modified pages) back to disk
    // This ensures that any changes made to pages in the buffer pool are saved before shutting down
    forceFlushPool(bm);

    // Iterate through all page frames to check if any are pinned
    for (int i = 0; i < buffer_size; i++)
    {
        // If fixCount (fCount) is not zero, it indicates that the page is currently in use
        // and has been modified by a client without being written back to disk
        if (Fpage[i].fCount != 0)
        {
            // Return an error if there are pinned pages in the buffer pool
            return RC_PINNED_PAGES_IN_BUFFER;
        }
    }

    // All pages are unpinned; safe to release the memory occupied by the page frames
    free(Fpage);
    
    // Set the management data pointer to NULL to avoid dangling references
    bm->mgmtData = NULL;

    return RC_OK;
}



extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
                          const int numPages, ReplacementStrategy strategy, 
                          void *stratData)
{
    // Assign the page file name and number of pages to the buffer pool structure
    bm->pageFile = (char *)pageFileName;
    bm->numPages = numPages;
    bm->strategy = strategy;

    // Allocate memory for the page frames in the buffer pool
    // Each Framep stores data for a page, so we allocate space based on the number of pages requested
    Framep *page = malloc(sizeof(Framep) * numPages);
    
    // Set the total number of pages in memory (buffer size) for future reference
    buffer_size = numPages;	
    int i;

    // Initialize all pages in the buffer pool to default values
    // This ensures all fields are set to a known state before use
    for(i = 0; i < buffer_size; i++)
    {
        page[i].data = NULL;         
        page[i].pageNum = -1;        
        page[i].bit_d = 0;           
        page[i].fCount = 0;          
        page[i].hitNum = 0;          
        page[i].refNum = 0;          
    }

    // Store the pointer to the allocated memory in the management data of the buffer pool
    bm->mgmtData = page;

    // Initialize global counters for write operations and clock pointer for replacement strategy
    wcount = clock_pointer = 0;
    
    return RC_OK;
}


extern RC forceFlushPool(BM_BufferPool *const bm)
{
    Framep *pageFrame = (Framep *)bm->mgmtData;
    
    int i;
    // Iterate through all page frames to store all dirty pages back to the disk
    for (i = 0; i < buffer_size; i++)
    {
        // Check if the page is clean (fCount == 0) and dirty (bit_d == 1)
        if (pageFrame[i].fCount == 0 && pageFrame[i].bit_d == 1)
        {
            SM_FileHandle fh;
            // Open the page file for writing
            openPageFile(bm->pageFile, &fh);
            // Write the block of data from the buffer to the page file on disk
            writeBlock(pageFrame[i].pageNum, &fh, pageFrame[i].data);
            // Mark the page as not dirty since it has been written back to disk
            pageFrame[i].bit_d = 0;
            wcount++;
        }
    }   
    return RC_OK;
}

extern RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{    
    Framep *pageFrame = (Framep *)bm->mgmtData;
    
    int i;
    // Iterate through all the pages in the buffer pool
    for (i = 0; i < buffer_size; i++)
    {
        // If the current page matches the page to be unpinned, decrease its fix count
        if (pageFrame[i].pageNum == page->pageNum)
        {
            pageFrame[i].fCount--; 
            break; 
        }        
    }
    return RC_OK;
}

extern RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    // Cast the management data to access the page frames
    Framep *pageFrame = (Framep *)bm->mgmtData;
    
    int i;
    // Iterate through all the pages in the buffer pool
    for (i = 0; i < buffer_size; i++)
    {
        // If the current page matches the page to be marked dirty
        if (pageFrame[i].pageNum == page->pageNum)
        {
            pageFrame[i].bit_d = 1; // Set the dirty bit to indicate the page has been modified
            return RC_OK; 
        }            
    }        
    return RC_ERROR;
}

extern RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    // Cast the management data to access the page frames
    Framep *pageFrame = (Framep *)bm->mgmtData;
    
    int i;
    // Iterate through all the pages in the buffer pool
    for (i = 0; i < buffer_size; i++)
    {
        // If the current page matches the page to be written to disk
        if (pageFrame[i].pageNum == page->pageNum)
        {        
            SM_FileHandle fh;
            // Open the page file for writing
            openPageFile(bm->pageFile, &fh);
            // Write the block of data from the buffer to the page file on disk
            writeBlock(pageFrame[i].pageNum, &fh, pageFrame[i].data);
        
            // Mark the page as not dirty since it has been successfully written to disk
            pageFrame[i].bit_d = 0;
            
            // Increment the write count to track the number of writes performed
            wcount++;
        }
    }   
    return RC_OK;
}


extern void FIFO(BM_BufferPool *const bm, Framep *page)
{
    Framep *pageFrames = (Framep *)bm->mgmtData;
    int frontIndex = rear_Index % buffer_size;

    // Search for an unpinned page frame to replace
    for (int i = 0; i < buffer_size; i++)
    {
        // Check if the page frame can be replaced (not pinned)
        if (pageFrames[frontIndex].fCount == 0)
        {
            // Write the dirty page back to disk if modified
            if (pageFrames[frontIndex].bit_d == 1)
            {
                SM_FileHandle fileHandle;
                openPageFile(bm->pageFile, &fileHandle);
                writeBlock(pageFrames[frontIndex].pageNum, &fileHandle, pageFrames[frontIndex].data);
                wcount++;
            }

            // Update the page frame with the new page's data
            pageFrames[frontIndex] = *page; // Copy new page data into the frame
            break;
        }
        // Move to the next page frame in FIFO order
        frontIndex = (frontIndex + 1) % buffer_size;
    }
}

extern void LRU(BM_BufferPool *const bm, Framep *page)
{
    Framep *pageFrames = (Framep *)bm->mgmtData;
    int leastHitIndex = -1;
    int leastHitNum = INT_MAX;

    // Find the first unpinned page frame
    for (int i = 0; i < buffer_size; i++)
    {
        if (pageFrames[i].fCount == 0)
        {
            leastHitIndex = i; // Initial unpinned page
            leastHitNum = pageFrames[i].hitNum; // Set initial least hit number
            break;
        }
    }

    // Iterate to find the least recently used page
    for (int i = (leastHitIndex + 1) % buffer_size; i != leastHitIndex; i = (i + 1) % buffer_size)
    {
        if (pageFrames[i].hitNum < leastHitNum)
        {
            leastHitIndex = i; // Update index for the least hit page
            leastHitNum = pageFrames[i].hitNum; // Update the least hit number
        }
    }

    // Write the dirty page back to disk if modified
    if (pageFrames[leastHitIndex].bit_d == 1)
    {
        SM_FileHandle fileHandle;
        openPageFile(bm->pageFile, &fileHandle);
        writeBlock(pageFrames[leastHitIndex].pageNum, &fileHandle, pageFrames[leastHitIndex].data);
        wcount++;
    }

    // Update the selected page frame with the new page's data
    pageFrames[leastHitIndex] = *page; // Copy new page data into the frame
}

extern void CLOCK(BM_BufferPool *const bm, Framep *page)
{
    Framep *pageFrames = (Framep *)bm->mgmtData;

    while (1)
    {
        // Wrap the clock pointer if it goes out of bounds
        clock_pointer = (clock_pointer % buffer_size);

        // Check for a candidate page frame to replace
        if (pageFrames[clock_pointer].hitNum == 0)
        {
            // Write the dirty page back to disk if modified
            if (pageFrames[clock_pointer].bit_d == 1)
            {
                SM_FileHandle fileHandle;
                openPageFile(bm->pageFile, &fileHandle);
                writeBlock(pageFrames[clock_pointer].pageNum, &fileHandle, pageFrames[clock_pointer].data);
                wcount++;
            }

            // Update the current page frame with the new page's data
            pageFrames[clock_pointer] = *page; // Copy new page data into the frame
            clock_pointer++; // Move the clock pointer to the next frame
            break;
        }
        else
        {
            // Reset the hit count for the recently used page
            pageFrames[clock_pointer++].hitNum = 0; 
        }
    }
}



// Function to get the contents of frames in the buffer pool
extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	PageNumber *frameContents = malloc(sizeof(PageNumber) * buffer_size);
	Framep *pageFrame = (Framep *) bm->mgmtData;
	
	int i = 0;
	// Iterating through all the pages in the buffer pool and setting frameContents' value to pageNum of the page
	while(i < buffer_size) {
		frameContents[i] = (pageFrame[i].pageNum != -1) ? pageFrame[i].pageNum : NO_PAGE;
		i++;
	}
	return frameContents;
}

extern RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
    Framep *pageFrames = (Framep *)bm->mgmtData;
    SM_FileHandle fileHandle;

    // Check if this is the first page being pinned (buffer is empty)
    if (pageFrames[0].pageNum == -1)
    {
        // Initialize the first page frame by reading from disk
        openPageFile(bm->pageFile, &fileHandle);
        pageFrames[0].data = (SM_PageHandle)malloc(PAGE_SIZE);
        ensureCapacity(pageNum, &fileHandle);
        readBlock(pageNum, &fileHandle, pageFrames[0].data);
        
        // Set up page frame attributes
        pageFrames[0].pageNum = pageNum;
        pageFrames[0].fCount = 1; // Increment fixCount for this page
        pageFrames[0].hitNum = 0; // Initialize hit number
        pageFrames[0].refNum = 0; // Reset reference count

        // Update the page handle with the new page
        page->pageNum = pageNum;
        page->data = pageFrames[0].data;

        return RC_OK;
    }
    
    bool bufferIsFull = true;

    // Iterate through the buffer pool to find the page
    for (int i = 0; i < buffer_size; i++)
    {
        if (pageFrames[i].pageNum != -1)
        {
            // If the page is already in the buffer
            if (pageFrames[i].pageNum == pageNum)
            {
                // Increment the fixCount and update hit count
                pageFrames[i].fCount++;
                bufferIsFull = false;
                hit++;

                // Update hitNum based on replacement strategy
                if (bm->strategy == RS_LRU)
                    pageFrames[i].hitNum = hit;
                else if (bm->strategy == RS_CLOCK)
                    pageFrames[i].hitNum = 1; // Indicate recently used

                // Set the page handle to the existing page
                page->pageNum = pageNum;
                page->data = pageFrames[i].data;

                clock_pointer++;
                return RC_OK; // Page found and pinned
            }
        }
        else
        {
            // The current page frame is empty; read the page from disk
            openPageFile(bm->pageFile, &fileHandle);
            pageFrames[i].data = (SM_PageHandle)malloc(PAGE_SIZE);
            readBlock(pageNum, &fileHandle, pageFrames[i].data);

            // Initialize the new page frame
            pageFrames[i].pageNum = pageNum;
            pageFrames[i].fCount = 1;
            pageFrames[i].refNum = 0;
            hit++;

            // Update hitNum based on replacement strategy
            if (bm->strategy == RS_LRU)
                pageFrames[i].hitNum = hit;
            else if (bm->strategy == RS_CLOCK)
                pageFrames[i].hitNum = 1; // Indicate recently used

            // Update the page handle
            page->pageNum = pageNum;
            page->data = pageFrames[i].data;

            bufferIsFull = false;
            return RC_OK; // Successfully pinned the page
        }
    }

    // If the buffer is full, we need to replace an existing page
    if (bufferIsFull)
    {
        // Allocate a new page frame
        Framep *newPage = (Framep *)malloc(sizeof(Framep));
        newPage->data = (SM_PageHandle)malloc(PAGE_SIZE);

        // Read the page from disk and initialize the new page frame
        openPageFile(bm->pageFile, &fileHandle);
        readBlock(pageNum, &fileHandle, newPage->data);
        newPage->pageNum = pageNum;
        newPage->fCount = 1;
        newPage->refNum = 0;
        hit++;

        // Update hitNum based on replacement strategy
        if (bm->strategy == RS_LRU)
            newPage->hitNum = hit;
        else if (bm->strategy == RS_CLOCK)
            newPage->hitNum = 1; // Indicate recently used

        // Update the page handle
        page->pageNum = pageNum;
        page->data = newPage->data;

        // Call the appropriate page replacement strategy
        switch (bm->strategy)
        {
            case RS_FIFO:
                FIFO(bm, newPage);
                break;
            case RS_LRU:
                LRU(bm, newPage);
                break;
            case RS_CLOCK:
                CLOCK(bm, newPage);
                break;
            default:
                printf("\nAlgorithm Not Implemented\n");
                break;
        }
    }

    return RC_OK;
}


// Function to retrieve an array indicating which pages are dirty in the buffer pool.
extern bool *getDirtyFlags (BM_BufferPool *const bm) {
    bool *dirtyFlags = (bool *) malloc(buffer_size * sizeof(bool));
    
    Framep *pageFrame = (Framep *) bm->mgmtData;
    
    for (int i = 0; i < buffer_size; i++) {
        // Set dirtyFlags[i] to true if page is dirty, otherwise false
        dirtyFlags[i] = (pageFrame[i].bit_d == 1);
    }
    
    return dirtyFlags; 
}

// Function to get the fix count of each page in the buffer pool.
extern int *getFixCounts (BM_BufferPool *const bm) {
    int *fixCounts = (int *) malloc(buffer_size * sizeof(int));
    
    Framep *pageFrame = (Framep *) bm->mgmtData;
    
    for (int i = 0; i < buffer_size; i++) {
        // Assign the fix count of each page; if unset (-1), set it to 0
        fixCounts[i] = (pageFrame[i].fCount != -1) ? pageFrame[i].fCount : 0;
    }
    
    return fixCounts; 
}

// Function to get the number of read I/O operations since buffer pool initialization.
extern int getNumReadIO (BM_BufferPool *const bm) {
    // Increment by one to adjust for starting from rear_Index = 0
    return (rear_Index + 1);
}

// Function to get the number of write I/O operations since buffer pool initialization.
extern int getNumWriteIO (BM_BufferPool *const bm) {
    return wcount; 
}
