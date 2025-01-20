**Assignment 1: Storage Manager**
	This project is an interface for a basic level storage manager. The functions outlined are for manipulating, reading, and writing page files.
functions 1-6: Haaniya Ghiasuddin
createPageFile:
Creates a new file and writes an empty page PAGE_SIZE, which is 4096 bytes, to initialize it. Returns an error if the file cannot be opened or if writing the empty page fails.
openPageFile:
Opens a file and initializes a file handle in order to manage it. The function calculates the number of pages in a file, and sets current page position to start. Returns an error if the file cannot be opened.
closePageFile:
Closes an open file using the handle. If it is already closed or not initialized, an error is returned. Once the file is closer, the function clears the file pointer.
destroyPageFile:
Deletes the specified file from the system. If the file does not exist it returns an error.
readBlock:
This function reads a specific page from a file into a memory buffer. First, the file pointer is retrieved from the file handle and checks if the file is open. If not, an error is returned. Then it checks to make sure the requested page is within range. Byte offset is calculated in the file corresponding to the page number. fseek is then used to move the pointer to this position. Next, the page is read into the memory buffer using fread. An error is returned if the number of bytes read is less than the page size. Lastly, the current page position in the file handle is updated.
functions 7-12: Aryan Nandu
getBlockPos()           This method returns the position of the block.

readFirstBlock()        This method returns position of the first page in the Page file.

readPreviousBlock()     It is used to read from previous block.

readCurrentBlock() 	    It is used to read from current block.

readNextBlock()         It is used to read from  Next Block. 

readLastBlock()         It is used to read from the Last Block.
functions 5-17: Zoha Saadia