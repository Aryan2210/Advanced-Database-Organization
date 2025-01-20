- Assignment-3:
    Name: Record Manager
    In the third assignment, we are tasked with developing a Database Record Manager with the help of 
    Assignment 1 (Storage Manager) and Assignment 2 (Buffer Manager).

- Team Information:

    Aryan Nandu          <anandu@hawk.iit.edu>          Hawk ID: A20583853
  
    Haaniya Ghiasuddin   <hghiasuddin1@hawk.iit.edu>    Hawk ID: A20485974
    
    Zoha Saadia          <zsaadia@hawk.iit.edu>         Hawk ID: A20482009

- Files:
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
    test_assign3_1.c
    test_expr.c
    test_helper.h

- Contents:
    1. Steps on how to run the code
    2. Description and usage of each function used in this assignment
 
- Instructions:
    1. **Navigate to Assignment Directory:**
   - Open the terminal and change the directory to 'Assignment3' using the command `cd assign_3`.

2. **Cleanup Previous Builds:**
   - Ensure a clean slate by executing `make clean` in the terminal to remove any existing 'test_assign3_1' file.

3. **Build Object Files:**
   - Generate object files by entering the command `make all` in the terminal.

4. **Run the Test Programs:**
   - Run the test programs using command `./test_assign3` in the terminal.
   - Run the test programs using command `./test_expr` in the terminal.



- Functions used:

**Initialize Record Manager (`initRecordManager()`):**
- Create the Record Manager from scratch using a specific entry from the buffer manager.

**Shutdown Record Manager (`shutdownRecordManager()`):**
- Release resources and shut down the Record Manager.

**Create Table (`createTable()`):**
- Creates the table with specified schema. 

**Open Table (`openTable()`):**
- Opens the table. 

**Close Table (`closeTable()`):**
- Closes the table, freeing any resources associated with it and marking it as unavailable for further operations.

**Delete Table (`deleteTable()`):**
- Deletes a table from storage, removing its data and associated metadata permanently.

**Get Number of Tuples (`getNumTuples()`):**
- Retrieves the number of tuples (records) currently stored in the table, providing a count of total records.

**Insert Record (`insertRecord()`):**
- Add a new row to the table.

**Delete Record (`deleteRecord()`):**
- Removes a specific record from the table based on its ID, freeing up space for future records.

**Update Record (`updateRecord()`):**
- Updates a record in the table.  

**Get Record (`getRecord()`):**
- Retreives a record by the id in the table. 

**Start Scan (`startScan()`):**
- Create a new scan manager and configure it with the table manager and provided expression.

**Next (`next()`):**
- Scans the record in the table and stores the next record. 

**Close Scan (`closeScan()`):**
- Remove the scan manager.

**Get Record Size (`getRecordSize()`):**
- Returns the size, in bytes, of a single record in the table, based on its schema.

**Create Schema (`createSchema()`):**
- Defines a schema for a table, setting up attributes, data types, and constraints for records to follow.

**Free Schema (`freeSchema()`):**
- Releases memory allocated for the schema, effectively deleting it from the system.

**Create Record (`createRecord()`):**
- Generate a blank record using the specified structure.

**Free Record (`freeRecord()`):**
- Remove a record.

**Get Attribute (`getAttr()`):**
- Retrieves the attribute from the given record in the specified schema. 

**Set Attribute (`setAttr()`):**
- Sets the attribute value in the record. 

**Search Function (`findFreeSlot()`):**
- This function returns a free slot within a page.