#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>

void initStorageManager (void){
}

RC createPageFile(char *fileName) {
    FILE *file = fopen(fileName, "w");// create/open file
    if (file == NULL) {
        return RC_FILE_NOT_FOUND;//returns error if not found
    }

    // Create an empty page with PAGE_SIZE bytes
    char emptyPage[PAGE_SIZE] = {0};// buffer allocated for one empty page
    size_t written = fwrite(emptyPage, 1, PAGE_SIZE, file);//write PAGE_SIZE bytes to file
    fclose(file);
    if (written != PAGE_SIZE) {
        return RC_WRITE_FAILED;//checks if page was written successfully. if not, return error.
    }
    return RC_OK;
}
RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    FILE *file = fopen(fileName, "r+");
    if (file == NULL) {
        return RC_FILE_NOT_FOUND;
    }
    fHandle->fileName = fileName;// set fileName
    fHandle->mgmtInfo = file;//store file pointer
    // Get the total number of pages
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fHandle->totalNumPages = fileSize / PAGE_SIZE;
    fHandle->curPagePos = 0;
    return RC_OK;
}
RC closePageFile(SM_FileHandle *fHandle) {
    FILE *file = (FILE *)fHandle->mgmtInfo;//gets file pointer
    if (file == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;//return error if file is not initialized
    }
    fclose(file);
    fHandle->mgmtInfo = NULL;//clear mgmtInfo
    return RC_OK;
}
RC destroyPageFile(char *fileName) {
    if (remove(fileName) != 0) {
        return RC_FILE_NOT_FOUND;//if file isn't found, return error
    }
    return RC_OK;
}

/* reading blocks from disc */
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    //check if the file handle is initialized
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT; //error handling, returns error if file handle is not initialized
    }
    //check if page number is within bounds
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE; //error if page does not exist
    }
    //seek to correct position in the file
    FILE *file = (FILE *) fHandle->mgmtInfo;// gets file pointer from handle
    int offset = pageNum * PAGE_SIZE;// calculate byte offset
    if (fseek(file, offset, SEEK_SET) != 0) {
        return RC_FILE_NOT_FOUND;//returns error if not found
    }
    //read page into memory
    size_t bytesRead = fread(memPage, sizeof(char), PAGE_SIZE, file);
    if (bytesRead < PAGE_SIZE) {
        return RC_READ_NON_EXISTING_PAGE;//returns error if reading less bytes than expected
    }
    //update the current page position
    fHandle->curPagePos = pageNum;

    return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle){
    return fHandle->curPagePos;
}
extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    // check if it has one page 
    if (fHandle->totalNumPages <= 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    // use the readBlock function to read the first blocK
    return readBlock(0, fHandle, memPage);
}
extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    // calculate the previous block index
    int previousBlockIndex = fHandle->curPagePos - 1;

    // check if the previousBlockIndex is valid (non-negative)
    if (previousBlockIndex < 0) {
        // invalid block index, cannot read the previous block
        return RC_READ_NON_EXISTING_PAGE;
    }

    // use the readBlock function to read the previous block
    return readBlock(previousBlockIndex, fHandle, memPage);
}
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    // Calculate the current block index based on the current page position
    int currentBlockIndex = fHandle->curPagePos;

    //Checking if the currentBlockIndex is valid
    if (currentBlockIndex < 0) {
        // Invalid block index, cannot read the current block
        return RC_READ_NON_EXISTING_PAGE;
    }

    // Use the readBlock function to read the current block
    return readBlock(currentBlockIndex, fHandle, memPage);
}
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    // Ensure the current page position is within valid bounds and there is a next page
    if (fHandle->curPagePos < 0 || fHandle->curPagePos >= fHandle->totalNumPages - 1) {
        // No next block exists, return error
        return RC_READ_NON_EXISTING_PAGE;
    }

    // Calculate the next block number 
    int nextBlockIndex = getBlockPos(fHandle) + 1;

    // Use the readBlock function to read the next block
    return readBlock(nextBlockIndex, fHandle, memPage);
}
extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    // Calculate the last block number (0-based index)
    int lastBlockIndex = fHandle->totalNumPages - 1;

    // Check if the lastBlockIndex is valid (non-negative)
    if (lastBlockIndex < 0) {
        // No pages in the file, cannot read the last block
        return RC_READ_NON_EXISTING_PAGE;
    }

    // Use the readBlock function to read the last block
    return readBlock(lastBlockIndex, fHandle, memPage);
}

/* writing blocks to a page file */
extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
	// Validate file handle
    if (fHandle == NULL || fHandle->fileName == NULL) {
        THROW(RC_FILE_HANDLE_NOT_INIT, "File handle is not initialized.");
    }

    FILE *file = fopen(fHandle->fileName, "r+b");
    if (file == NULL) {
        THROW(RC_FILE_NOT_FOUND, "File not found.");
    }

    // Check page number validity
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        fclose(file);
        THROW(RC_READ_NON_EXISTING_PAGE, "Page number out of bounds.");
    }

    // Seek to the correct position
    if (fseek(file, pageNum * PAGE_SIZE, SEEK_SET) != 0) {
        fclose(file);
        THROW(RC_WRITE_FAILED, "Failed to seek to the page.");
    }

    // Write the page
    if (fwrite(memPage, PAGE_SIZE, 1, file) != 1) {
        fclose(file);
        THROW(RC_WRITE_FAILED, "Failed to write the page.");
    }

    fclose(file);
    return RC_OK;
}
extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}
extern RC appendEmptyBlock (SM_FileHandle *fHandle){
    // Validate 
    if (fHandle == NULL || fHandle->fileName == NULL) {
        THROW(RC_FILE_HANDLE_NOT_INIT, "File handle is not initialized.");
    }

    FILE *file = fopen(fHandle->fileName, "a+b");
    if (file == NULL) {
        THROW(RC_FILE_NOT_FOUND, "File not found.");
    }

    // Create empty page
    SM_PageHandle emptyPage = (SM_PageHandle)calloc(1, PAGE_SIZE);
    if (emptyPage == NULL) {
        fclose(file);
        THROW(RC_WRITE_FAILED, "Failed to allocate memory for an empty page.");
    }

    // Write empty page
    if (fwrite(emptyPage, PAGE_SIZE, 1, file) != 1) {
        free(emptyPage);
        fclose(file);
        THROW(RC_WRITE_FAILED, "Failed to write an empty page.");
    }

    free(emptyPage);
    fHandle->totalNumPages++;
    fclose(file);
    return RC_OK;
}
extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    // Validate 
    if (fHandle == NULL || fHandle->fileName == NULL) {
        THROW(RC_FILE_HANDLE_NOT_INIT, "File handle is not initialized.");
    }

    // If the current num pages enough
    if (fHandle->totalNumPages >= numberOfPages) {
        return RC_OK;
    }

    FILE *file = fopen(fHandle->fileName, "a+b");
    if (file == NULL) {
        THROW(RC_FILE_NOT_FOUND, "File not found.");
    }

    // Append 
    int pagesToAppend = numberOfPages - fHandle->totalNumPages;
    for (int i = 0; i < pagesToAppend; i++) {
        SM_PageHandle emptyPage = (SM_PageHandle)calloc(1, PAGE_SIZE);
        if (emptyPage == NULL) {
            fclose(file);
            THROW(RC_WRITE_FAILED, "Failed to allocate memory for an empty page.");
        }

        if (fwrite(emptyPage, PAGE_SIZE, 1, file) != 1) {
            free(emptyPage);
            fclose(file);
            THROW(RC_WRITE_FAILED, "Failed to write an empty page.");
        }

        free(emptyPage);
    }

    fHandle->totalNumPages = numberOfPages;
    fclose(file);
    return RC_OK;
}

