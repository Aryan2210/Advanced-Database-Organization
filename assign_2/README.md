- Assignment-2:
    Name: Buffer Manager
    In the second assignment we will accomplish a Buffer Manager of a Database with the help of Assignment 1 - Storage Manager.

- Team Information:

    Aryan Nandu          <anandu@hawk.iit.edu>          Hawk ID: A20583853
  
    Haaniya Ghiasuddin   <hghiasuddin1@hawk.iit.edu>    Hawk ID: A20485974
    
    Zoha Saadia          <zsaadia@hawk.iit.edu>         Hawk ID: 

- Files:
    Makefile
    README.md
    buffer_mgr_stat.c
    buffer_mgr_stat.h
    buffer_mgr.c
    buffer_mgr.h
    dberror.c
    dberror.h
    dt.h
    storage_mgr.c
    storage_mgr.h
    test_assign2_1.c
    test_assign2_2.c
    test_helper.h

- Contents:
    1. Steps on how to run the code
    2. Description and usage of each function used in this assignment
 
- Instructions:
    1. Change the directory of the assignment using terminal to assign2_storage manager
    2. Use clean command to remove existing .o files present in your system by typing "make clean".
    3. Then run the "make" command to make the object files and run those file .
    
- Functions used:

Name                    Description
______________________________________________________________________________________________________
createPageFrame:    	This function is used to create a new page frame. It allocates memory for a new   
                        frame, initializes its properties, and appends it to the linked list of frames in the buffer pool.

shutdownBufferPool:     This function releases all resources associated with the buffer pool.

initBufferPool:         This function initializes a buffer pool.

forceFlushPool: 	    This function writes all dirty pages in the buffer pool back to disk, ensuring that any modifications made to the pages are saved.

unpinPage: 		        This function is used to unpin a page, which means decreasing its fix count.

markDirty:              This function marks a specified page as dirty, indicating that the page has been modified and needs to be written back to disk before being evicted

forcePage:              This function immediately writes a specific page back to disk

pinPageFIFO: 		    This function implements the FIFO page replacement strategy.

pinPageLRU: 		    This function implements the LRU page replacement strategy.

pinPageCLOCK: 		    This function implements the CLOCK page replacement strategy.

getFrameContents: 	    This function returns an array containing the page numbers stored in the frames.

getDirtyFlags: 		    This function returns an array of flags indicating whether each page in the buffer  
                        pool is dirty.

getFixCounts: 		    This function returns an array containing the fix counts for each page in the buffer 
                        pool.