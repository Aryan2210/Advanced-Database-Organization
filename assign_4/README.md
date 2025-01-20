- Assignment-4:
    Name: Index Manager
    In the fourth assignment, we are tasked with developing a Database Record Manager with the help of 
    Assignment 1 (Storage Manager), Assignment 2 (Buffer Manager), Assignment 3 (Record Manager).

- Team Information:

    Aryan Nandu          <anandu@hawk.iit.edu>          Hawk ID: A20583853
  
    Haaniya Ghiasuddin   <hghiasuddin1@hawk.iit.edu>    Hawk ID: A20485974
    
    Zoha Saadia          <zsaadia@hawk.iit.edu>         Hawk ID: A20482009


- Files:
    btree_mgr.c
    btree_mgr.h
    buffer_mgr_stat.c
    buffer_mgr_stat.h
    buffer_mgr.c
    buffer_mgr.h
    dberror.c
    dberror.h
    dt.h
    expr.c
    expr.h
    Makefile
    record_mgr.c
    record_mgr.h
    rm_serializer.c
    storage_mgr.c
    storage_mgr.h
    tables.h
    test_assign4_1.c
    test_expr.c
    test_helper.h

- Contents:
    1. Steps on how to run the code
    2. Description and usage of each function used in this assignment
 
- Instructions:
    1. **Navigate to Assignment Directory:**
   - Open the terminal and change the directory to 'Assignment3' using the command `cd assign_4`.

2. **Cleanup Previous Builds:**
   - Ensure a clean slate by executing `make clean` in the terminal to remove any existing 'test_assign4_1' 
     file.

3. **Build Object Files:**
   - Generate object files by entering the command `make test_assign4_1` in the terminal.
   - Generate object files by entering the command `make test_expr` in the terminal.

4. **Run the Test Programs:**
   - Run the test programs using command `make test1` in the terminal.
   - Run the test programs using command `make test2` in the terminal.

- Functions used:

### Functions in `btree_mgr.c`

1. **initIndexManager**: Initializes the index manager by setting up necessary resources like the storage manager.

2. **createBtree**: Creates a new B+-Tree index with specified key type and fan-out, initializes its root node, and stores it in a page file.

3. **shutdownIndexManager**: Shuts down the index manager by releasing allocated resources.

4. **deleteBtree**: Deletes an existing B+-Tree by freeing all its nodes, closing its buffer pool, and destroying its page file.

5. **openBtree**: Opens an existing B+-Tree index, loads its metadata and root node from a page file.

6. **closeBtree**: Closes a B+-Tree index, saves its metadata and root node to the page file, and frees allocated memory.

7. **getNumNodes**: Counts the total number of nodes in the B+-Tree.

8. **getNumEntries**: Counts the total number of entries (keys) in the B+-Tree.

9. **getKeyType**: Retrieves the data type of the keys used in the B+-Tree.

10. **findKey**: Searches for a specific key in the B+-Tree and retrieves its associated record ID (RID) if found.

11. **insertKey**: Inserts a new key and its associated RID into the B+-Tree, splitting nodes if necessary.

12. **deleteKey**: Deletes a key from the B+-Tree and handles underflows in leaf nodes if needed.

13. **openTreeScan**: Initiates a sequential scan of the B+-Tree, starting from the leftmost leaf node.

14. **nextEntry**: Retrieves the next key and RID during a sequential scan of the B+-Tree.

15. **closeTreeScan**: Terminates a sequential scan and frees associated resources.

16. **printNode**: Prints single node.  

17. **traverseTree**: Recursive function to traverse tree.  

18. **printTree**: Prints the structure of the B+-Tree, showing all its nodes and keys in a human-readable format.


### Helper Functions:

1. **SeperatorForInt**: Parses interger from string up to the specified delimeter. 

2. **readMD**: Reads metadata from page and creates fileMD structure.

3. **SeperatorForFloat**: Parses float from string up to the specified delimiter. 

4. **locatePGtoInsert**: Finds page to insert a key in the B-tree. 

5. **NewKeyPtr**: Inserts a new key and pointer into pgData. 

6. **aSpace**: Allocates memory for string and initialization

7. **dSpace**: Frees the memory for string 

8. **formateDataKeyPtr**: Formats key-pointer data into string. 

9. **pMetaData**: Creates metadata string for index tree. 

10. **PageDataToWrite**: Formats page data into a string for writing. 

11. **pWrite**: Writes formatted content to a page. 

12. **propogateUp**: Propogates changes up in B-Tree after insertion.

13. **updateChildNodesOfParentDown**: Updates parent node info for child nodes. 

### Contributions:

Aryan:btree_mgr.c (1-6), helper functions (1-4)

Haaniya:btree_mgr.c (7-11), helper functions (5-9)

Zoha:btree_mgr.c (12-18), helper functions (10-13)