#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "buffer_mgr_stat.h"
#include "dberror.h"
#include "storage_mgr.h"
#include "btree_mgr.h"
#include "buffer_mgr.h"

//************************************Data Structures*************************************

//Key Data, it has key and lside and rside pointers
typedef struct data{

    float lside;
    float rside;
    int key;

}data;

//Meta data of the file
typedef struct fileMD{
    
    int kType; // optional value
    int maxEntriesPpage;
    int nOfEntries;     //Total number of entries in the file
    int numberOfNodes; // number of page nodes
    int rootPageNumber;
    
}fileMD;

//Mgmt Data
typedef struct treeData{

    BM_BufferPool* Bmanager;
    SM_FileHandle flHandler;
    BM_PageHandle* pgHandler;
    fileMD fMD;
   

}treeData;

//Page Data
typedef struct pgData{
    

    float *pointers; //n+1 pointers for all pages
    int *keys; //n entries for all pages 
    int numberEntries;
    int pgNumber;
    int leaf;
    int parentnode;
   
    
}pgData;

//Scan Management Data
typedef struct scanData{

    int curPage;//Page Number of current page
    int isCurPageLoaded;
    int nextPagePosInLeafPages;
    int noOfLeafPage;
    int curPosInPage;
    int *leafPage; //entry of leaf pages 
    pgData curPageData;
    
}scanData;


//Global variables
treeData* btreeMgmt;
static int counter = 0;       
scanData* scMgmtData;
BTreeHandle* TreeHandler;   
BT_ScanHandle* scanHandle;


//****************************************************************************************


//************************************Helper method prototype*****************************

int SeperatorForInt(char **ptr, char c);
RC readMD(BM_BufferPool* bm,BM_PageHandle* ph,fileMD* fmd,int pageNumber);
float SeperatorForFloat(char **ptr, char c);
RC readPageData(BM_BufferPool* Bmanager, BM_PageHandle* pgHandler, pgData* pgData, int pageNumber);
pgData locatePGtoInsert(BM_BufferPool* Bmanager, BM_PageHandle* pgHandler, pgData root, int key);
RC NewKeyPtr(pgData* pageData, int key, RID rid);
RC aSpace(char **data);
RC dSpace(char **data);
RC formateDataKeyPtr(pgData* pd, char* data);
RC pMetaData(fileMD* fmd,char* content);
RC PageDataToWrite(pgData* pd,char* content);
RC pWrite(BM_BufferPool* bm,BM_PageHandle* ph,char* content,int pageNumber);
RC propogateUp(BTreeHandle *tree,int pageNumber,data kd);
RC updateChildNodesOfParentDown(BTreeHandle* tree,pgData node);
RC insertKeyAndPointerInNonLeaf(pgData* page,data kd);
RC keyAndPointerDeletingInLeaf(pgData* pg, int key);
RC getLeafPg(pgData root,BM_BufferPool* bm,BM_PageHandle* ph,int* leafPages);


//****************************************************************************************


//************************************Initiate and Shutdown*******************************

extern RC initIndexManager (void *mgmtData){
    printf("*******Index Manager Intialized:*******");
    return RC_OK;
}

extern RC shutdownIndexManager (){
    return RC_OK;
}

//****************************************************************************************


//************************************create, destroy, open, and close an btree index*****

//Create brtree
RC createBtree (char *idxId, DataType kType, int n) {
    //printf("create b tree");

    btreeMgmt = ( treeData*)malloc (sizeof(treeData) );
    TreeHandler = ( BTreeHandle*)malloc (sizeof(BTreeHandle) );
    scanHandle = ( BT_ScanHandle*)malloc (sizeof(BT_ScanHandle) );
    scMgmtData = ( scanData*)malloc (sizeof(scanData) );
    
    // initalising buffer pool, page handler and scan handler
    btreeMgmt -> Bmanager = MAKE_POOL();
    btreeMgmt -> pgHandler = MAKE_PAGE_HANDLE();
    TreeHandler -> mgmtData = btreeMgmt;
    scanHandle -> mgmtData = scMgmtData;

    //scMgmtData = ( scanData*)malloc (sizeof(scanData) ); 
    createPageFile(idxId); // creating page file

    int rt_val = openPageFile(idxId, &(btreeMgmt->flHandler));
    //printf("page file created");
    
    // setting the intital btree metadata
    btreeMgmt ->fMD.numberOfNodes = 1;
    btreeMgmt ->fMD.rootPageNumber = 1;
    btreeMgmt ->fMD.nOfEntries = 0;
    btreeMgmt ->fMD.maxEntriesPpage = n;

    initBufferPool(btreeMgmt -> Bmanager, idxId, 10, RS_FIFO, NULL);
    ensureCapacity(2,&(btreeMgmt -> flHandler)); // to ensure that 2 pages are present

    char *data_str;

    //printf("buffer pool initalized ");
    aSpace( &data_str );
    pMetaData( &(btreeMgmt -> fMD ), data_str );
    pWrite( btreeMgmt -> Bmanager, btreeMgmt -> pgHandler, data_str, 0 );
    dSpace(&data_str);

    //printf("page written");

    aSpace( &data_str );
    pgData root;

    root.pgNumber = 1;
    root.leaf = 1;
    root.parentnode = -1;
    root.numberEntries = 0;    

    aSpace( &data_str );
    PageDataToWrite( &root, data_str );
    pWrite( btreeMgmt -> Bmanager, btreeMgmt -> pgHandler, data_str, btreeMgmt ->fMD.rootPageNumber );
    //printf("page writted!");
    dSpace( &data_str );
    
    shutdownBufferPool(btreeMgmt->Bmanager); // shutdown the buffer pool
    //printf("b tree created");
    
    return RC_OK;
}

//Open btree
extern RC openBtree (BTreeHandle **tree, char *idxId){
    int rt_val = openPageFile( idxId, &(btreeMgmt -> flHandler) ); // open the page file
    
    //printf("file opened");

    if (rt_val != RC_OK) { // check if file is opened
        return rt_val; 
    }

    // intialise the buffer manager and page handler
    btreeMgmt -> Bmanager = MAKE_POOL();
    btreeMgmt -> pgHandler = MAKE_PAGE_HANDLE();
    initBufferPool( btreeMgmt -> Bmanager, idxId,10, RS_FIFO,NULL );

    //printf("buffer pool initialized");

    // reading meta data
    fileMD fmd;
    readMD( btreeMgmt -> Bmanager, btreeMgmt -> pgHandler, &fmd,0);

    //printf("read meta data");

    // adding the data to tree handler and b tree manager
    TreeHandler -> idxId = idxId;
    TreeHandler -> kType = fmd.kType;
    btreeMgmt -> fMD.numberOfNodes = fmd.numberOfNodes;
    btreeMgmt -> fMD.maxEntriesPpage = fmd.maxEntriesPpage;
    btreeMgmt -> fMD.rootPageNumber = fmd.rootPageNumber;
    btreeMgmt -> fMD.nOfEntries = fmd.nOfEntries;

    TreeHandler -> mgmtData = btreeMgmt;
    *tree = TreeHandler;

    return RC_OK;
}

//Close
extern RC closeBtree (BTreeHandle *tree){
    // getting the buffer manager
    BM_BufferPool *bm = ((treeData*)tree->mgmtData)->Bmanager;
    // BM_PageHandle *ph = ((treeData*)tree->mgmtData)-> pgHandler;
    // SM_FileHandle fh = ((treeData*)tree->mgmtData)->flHandler;

    char *data_str;

    // wrting the data into disk before closing the buffer
    aSpace( &data_str );
    pMetaData( &(btreeMgmt->fMD), data_str );
    pWrite( btreeMgmt -> Bmanager, btreeMgmt -> pgHandler, data_str, 0 );
    dSpace( &data_str );
    shutdownBufferPool( bm );
    
    // freeing the space
    free( btreeMgmt->Bmanager );
    free( btreeMgmt->pgHandler );
    free( tree->mgmtData );
    free(tree);
    //free( TreeHandler );

    return RC_OK;
}

//Delete tree
extern RC deleteBtree (char *idxId){
    remove(idxId);

    return RC_OK;
}

//****************************************************************************************


//************************************Access information about a b-tree*******************

RC getNumNodes (BTreeHandle *tree, int *result){ // read the num of nodes
    *result = ((treeData*)tree->mgmtData) -> fMD.numberOfNodes;
    return RC_OK;
}

RC getkType(BTreeHandle *tree, DataType *result){ // get the key type
    *result=((treeData*)tree->mgmtData)->fMD.kType;
    return RC_OK;
}

RC getNumEntries(BTreeHandle *tree, int *result){ // get number of entries
    *result=((treeData*)tree->mgmtData)->fMD.nOfEntries;
    return RC_OK;
}

//****************************************************************************************

//***************************************Helper functions****************************

int SeperatorForInt(char **ptr, char c){
    char *tempPtr=*ptr;
    char *val=(char*)malloc(100);
    //printf("\nstart int seperator\n");

    memset(val,'\0',sizeof(val));

    for(int i=0;*tempPtr!=c;i++,tempPtr++){
        val[i]=*tempPtr;
    }

    int val2=atoi(val);
    *ptr=tempPtr;
    free(val);
    
    //printf("\nend int seperator\n");

    return val2;
    
}

RC readMD(BM_BufferPool* Bmanager,BM_PageHandle* pgHandler,fileMD* fMD,int pgNumber){
    //Read the index metadata from page 0 
    pinPage(Bmanager,pgHandler,pgNumber);

    char *cursor = pgHandler->data;

    //printf("I am here");
    cursor++; // Skip initial marker
    // Extract the data between first $ and next $ : root node's page number
    fMD->rootPageNumber = SeperatorForInt(&cursor,'$');

    //printf("I am done");
    cursor++; // Skip the next section
    fMD->numberOfNodes = SeperatorForInt(&cursor,'$');

    cursor++; // Skip the next section
    fMD->nOfEntries = SeperatorForInt(&cursor,'$');

    cursor++; // Skip the next section
    fMD->maxEntriesPpage = SeperatorForInt(&cursor,'$');

    cursor++; // Skip the next section
    fMD->kType = SeperatorForInt(&cursor,'$');

    unpinPage(Bmanager,pgHandler);

    return RC_OK;
}


float SeperatorForFloat(char **ptr, char c){
    char *tempPtr = *ptr;
    char *val=(char*)malloc(100);

    //printf("\nstart float seperator\n");

    memset(val,'\0',sizeof(val));
    for(int i=0;*tempPtr!=c;tempPtr++,i++){
        val[i]=*tempPtr;
    }
    
    //printf("\nloop start!\n");
    // int index=0;
    // while(*tempPtr!=c){
    //     val[index]=*tempPtr;
    //     tempPtr++;
    //     index++;
    // }
    //printf("\nloop done!\n");
    
    *ptr=tempPtr;
    float val2=atof(val);
    free(val);

   // printf("\nend float seperator\n");

    return val2;
}

RC readPageData(BM_BufferPool* Bmanager, BM_PageHandle* pgHandler, pgData* pgData, int pageNumber){
    
    pinPage(Bmanager,pgHandler,pageNumber); // pinning the page

    char *pgHandlerData=pgHandler->data;
    pgHandlerData++; // skip the inital character

    //printf("\nskip initial character\n");

    // reading data
    pgData->leaf =SeperatorForInt(&pgHandlerData,'$');
    pgHandlerData++;

    pgData->numberEntries =SeperatorForInt(&pgHandlerData,'$');
    pgHandlerData++;
    
    pgData->parentnode =SeperatorForInt(&pgHandlerData,'$');
    pgHandlerData++;

    pgData->pgNumber =SeperatorForInt(&pgHandlerData,'$');
    pgHandlerData++;

    //printf("\ndone seperating!\n");

    int index=0;
    float *children=malloc((pgData->numberEntries+1)*sizeof(float));
    int *key=malloc(pgData->numberEntries*sizeof(int));

    if(pgData->numberEntries>0){
        while(index<pgData->numberEntries){
            children[index] = SeperatorForFloat(&pgHandlerData,'$');
            pgHandlerData++;
            key[index] = SeperatorForInt(&pgHandlerData,'$');
            pgHandlerData++;
            index++;
        }
        children[index]=SeperatorForFloat(&pgHandlerData,'$');
    }

    //printf("\ndone final seprate!\n");

    pgData->pointers=children;
    pgData->keys=key;
    unpinPage(Bmanager,pgHandler);

    return RC_OK;
}

pgData locatePGtoInsert(BM_BufferPool* Bmanager, BM_PageHandle* pgHandler, pgData root, int key){
    if(root.leaf){
        return root;
    }
    else{
        if(key<root.keys[0]){
            int pageSearchNumber = round(root.pointers[0]*10)/10;
            pgData searchPage;
            readPageData(Bmanager,pgHandler,&searchPage,pageSearchNumber);
            return locatePGtoInsert(Bmanager,pgHandler,searchPage,key);
        }
        else{
            int foundPage=0;
            size_t index;
            for(index=0; index < root.numberEntries-1;index++){
                if(key>=root.keys[index] && key<root.keys[index+1]){
                    pgData searchInPage;
                    foundPage=1;
                    int pageSearchNumber=round(root.pointers[index+1]*10)/10;
                    readPageData(Bmanager,pgHandler,&searchInPage,pageSearchNumber);
                    return locatePGtoInsert(Bmanager,pgHandler,searchInPage,key);
                }
                //index++;
            }
            if(!foundPage){
                int pageSearchNumber = round(root.pointers[root.numberEntries]*10)/10;
                pgData searchPage;
                readPageData(Bmanager,pgHandler,&searchPage,pageSearchNumber);
                return locatePGtoInsert(Bmanager,pgHandler,searchPage,key);
            }
        }
    }
}

RC NewKeyPtr(pgData* pageData, int key, RID rid)
{
    float *children=(float*)malloc(sizeof(int)*10);
    int *keys=(int*)malloc(sizeof(int)*10);
    
    int index=0;
    while(index< pageData->numberEntries&& key> pageData->keys[index]){
        keys[index]=pageData->keys[index];
        children[index] = pageData->pointers[index];
        index++;
    }

    if(index<pageData->numberEntries && key == pageData->keys[index]){
        return RC_IM_KEY_ALREADY_EXISTS;
    }
    else{
        float cPointer= rid.page+ rid.slot*0.1;
        keys[index]= key;
        children[index] = cPointer;
        index++;
    }

    while(index < pageData->numberEntries+1){
        children[index]=pageData->pointers[index-1];
        keys[index]= pageData->keys[index-1];
        index++;
    }

    children[index]=-1;
    free(pageData->keys);
    free(pageData->pointers);
    pageData->keys= keys;
    pageData->pointers=children;
    pageData->numberEntries++;
    return RC_OK;
}

RC aSpace(char **data){
    *data=(char*)malloc(50*sizeof(char));
    memset(*data,'\0',50);
    //printf("space allocated");
}

RC dSpace(char **data){
    free(*data);
    //printf("space deallocated");
}
RC formateDataKeyPtr(pgData* pgData, char* content){
    //printf("key pointer formated data");
    char *cursor = content;
    int index = 0;
    
    while(index < pgData->numberEntries) {
        int childValue = round(pgData->pointers[index]*10);
        int slot =  childValue % 10;
        int pageNum =  childValue / 10;

        sprintf(cursor,"%d.%d$",pageNum,slot);
        cursor += 4;
        sprintf(cursor,"%d$",pgData->keys[index]);
        if(pgData->keys[index] >=10){
            cursor += 3;
        }
        else{
            cursor += 2;
        }
        index++;
    }
    sprintf(cursor,"%0.1f$",pgData->pointers[index]);
    //printf("key pointer formated!");
}

RC pMetaData(fileMD* fMD,char* data){
    //Create metadata for this index tree
    sprintf (data,"$%d$%d$%d$%d$%d$",fMD->rootPageNumber,fMD->numberOfNodes,fMD->nOfEntries,fMD->maxEntriesPpage,fMD->kType);
}


RC PageDataToWrite(pgData* pageData,char* content){
    sprintf (content,"$%d$%d$%d$%d$",pageData->leaf,pageData->numberEntries,pageData->parentnode,pageData->pgNumber);
    //printf("number of entries: %d",pd->numberEntries);
    if(pageData->numberEntries > 0){
        char* keysAndPointers;
        aSpace(&keysAndPointers);
        formateDataKeyPtr(pageData,keysAndPointers);
        sprintf (content+strlen(content),"%s",keysAndPointers);
        //sprintf(content + offset, "%s", keysAndPointers); // Append formatted key-pointer data to content
        dSpace(&keysAndPointers);
    }
    //printf("page prepared");
}

RC pWrite(BM_BufferPool* Bmanager,BM_PageHandle* pgHandler,char* content,int pageNumber){
    //printf("page write\n");
    // Pin the page with specified index to modify its contents
    pinPage(Bmanager,pgHandler,pageNumber);
    //printf("page pinned");
    // Clear existing data in the page buffer and set new content
    memset(pgHandler->data,'\0',100); // Clear up to 100 characters
    sprintf(pgHandler->data,"%s",content); // Copy new data into the page
    // Mark the page as dirty since its content has been changed
    markDirty(Bmanager,pgHandler);
    unpinPage(Bmanager,pgHandler); 
}

RC propogateUp(BTreeHandle *treeHandler,int pgNumb,data keyData){

    BM_BufferPool *Bmanager = ((treeData*)treeHandler->mgmtData)->Bmanager;
    BM_PageHandle *pgHandler = ((treeData*)treeHandler->mgmtData)->pgHandler;
    SM_FileHandle flHandler = ((treeData*)treeHandler->mgmtData)->flHandler;
    
    int maxEntity = ((treeData*)treeHandler->mgmtData)->fMD.maxEntriesPpage;
    int curNumOfNodes = ((treeData*)treeHandler->mgmtData)->fMD.numberOfNodes;
    
    // if parent page number is -1, create new node
    if(pgNumb != -1){
        pgData newPageToAdd; // page is full or not full, add data in existing page
        readPageData(Bmanager,pgHandler,&newPageToAdd,pgNumb);
        insertKeyAndPointerInNonLeaf(&newPageToAdd,keyData);
        
        if(newPageToAdd.numberEntries > maxEntity){ // if page have more than max entries
            // if is true, divide and copy the data into new pages, the key of new node will be in rside child and old node will be in lside child. 
            float *oldNodeChildren = (float *)malloc(10*sizeof(float));
            int count = 0, *oldNodeKeys = (int *)malloc(10*sizeof(int));
            size_t index = 0;
            while(index < (int)ceil((newPageToAdd.numberEntries)/2))
            {
                oldNodeKeys[count] = newPageToAdd.keys[index];
                oldNodeChildren[count] = newPageToAdd.pointers[index];
                count++;
                index++;
            }
            oldNodeChildren[count] = newPageToAdd.pointers[count];
            count++;

            int count2 = 0,*keysForNewNode = (int *)malloc(10*sizeof(int));
            float *childrenForNewNode = (float *)malloc(10*sizeof(float));
            index=count;
            while(index < newPageToAdd.numberEntries + 2)
            {
                keysForNewNode[count2] = newPageToAdd.keys[index];
                childrenForNewNode[count2] = newPageToAdd.pointers[index];
                count2++;
                index++;
            }
            childrenForNewNode[count2] = newPageToAdd.pointers[count];

            ensureCapacity (curNumOfNodes + 2, &flHandler);

            curNumOfNodes += 1;

            ((treeData*)treeHandler->mgmtData)->fMD.numberOfNodes++;

            //rside child data
            pgData pRChild;
            pRChild.leaf = 0;
            pRChild.pgNumber = curNumOfNodes;
            pRChild.numberEntries = (int)floor((maxEntity+1)/2);
            pRChild.keys = keysForNewNode;
            pRChild.pointers = childrenForNewNode;
            pRChild.parentnode = newPageToAdd.parentnode;

            //data update on rside child
            char *dataStr;
            aSpace(&dataStr);
            PageDataToWrite(&pRChild,dataStr);
            pWrite(Bmanager,pgHandler,dataStr,pRChild.pgNumber);
            dSpace(&dataStr);

            //lside child Data
            pgData pLChild;
            pLChild.leaf = 0;
            pLChild.pgNumber = newPageToAdd.pgNumber;
            pLChild.numberEntries = (int)floor((maxEntity+1)/2);
            pLChild.keys = oldNodeKeys;
            pLChild.pointers = oldNodeChildren;
            pLChild.parentnode = newPageToAdd.parentnode;

            //Update data on lside child
            aSpace(&dataStr);
            PageDataToWrite(&pLChild,dataStr);
            pWrite(Bmanager,pgHandler,dataStr,pLChild.pgNumber);
            dSpace(&dataStr);

            int pgNum = newPageToAdd.parentnode;
            float lside = pLChild.pgNumber;
            float rside = pRChild.pgNumber;

            data kdata;
            kdata.key = newPageToAdd.keys[(int)ceil((maxEntity+1)/2)];
            kdata.lside = lside;
            kdata.rside = rside;

            //update childe of each parent
            updateChildNodesOfParentDown(treeHandler,pRChild);

            //propagate up
            propogateUp(treeHandler,pgNum,kdata);

        }
        else{
            char *dataString;
            aSpace(&dataString);
            PageDataToWrite(&newPageToAdd,dataString);
            pWrite(Bmanager,pgHandler,dataString,newPageToAdd.pgNumber);
            dSpace(&dataString);
            return RC_OK;   
        }

    }
    else{
        int curNumOfNodes = ((treeData*)treeHandler->mgmtData)->fMD.numberOfNodes;
        ensureCapacity(curNumOfNodes+2,&flHandler);

        pgData newRoot; // making new node as root
        newRoot.pgNumber = curNumOfNodes+1;

        int *keysofNewRoot = (int *)malloc(10*sizeof(int));
        keysofNewRoot[0] = keyData.key;

        float *childrenofNewRoot = (float *)malloc(10*sizeof(float));
        childrenofNewRoot[0] = keyData.lside;
        childrenofNewRoot[1] = keyData.rside;

        newRoot.keys = keysofNewRoot;
        newRoot.pointers= childrenofNewRoot;
        newRoot.numberEntries = 1;
        newRoot.parentnode = -1;
        newRoot.leaf = 0;

        ((treeData*)treeHandler->mgmtData)->fMD.numberOfNodes++;
        ((treeData*)treeHandler->mgmtData)->fMD.rootPageNumber = newRoot.pgNumber;

        char *dataString;
        aSpace(&dataString);
        PageDataToWrite(&newRoot,dataString);
        pWrite(Bmanager,pgHandler,dataString,newRoot.pgNumber);
        dSpace(&dataString);

        //updating child nodes of each parent
        updateChildNodesOfParentDown(treeHandler,newRoot);
    }
}

RC updateChildNodesOfParentDown(BTreeHandle* tree,pgData dataNode){

    BM_BufferPool *Bmanager = ((treeData*)tree->mgmtData)->Bmanager;
    BM_PageHandle *pgHandler = ((treeData*)tree->mgmtData)->pgHandler;
    
    // int numChildren = node.numberEntries+1;
    size_t index = 0;

    while(index < dataNode.numberEntries+1) {
        pgData child;
        readPageData(Bmanager,pgHandler,&child,dataNode.pointers[index]);

        //Update the parent
        child.parentnode = dataNode.pgNumber;

        char *data;
        aSpace(&data);
        PageDataToWrite(&child,data);
        pWrite(Bmanager,pgHandler,data,child.pgNumber);
        dSpace(&data);
        index++;
    }

    return RC_OK;

};



RC insertKeyAndPointerInNonLeaf(pgData* pgData,data keyData){
    int *updatedKeys = (int*)malloc(sizeof(int)*10); // Array for new keys
    float *updatedPointers = (float*)malloc(sizeof(int)*10); // Array for new pointers
    int currentPosition = 0;
    while(keyData.key > pgData->keys[currentPosition] && currentPosition < pgData->numberEntries){
        updatedKeys[currentPosition] = pgData->keys[currentPosition];
        updatedPointers[currentPosition] = pgData->pointers[currentPosition];
        currentPosition++;
    }

    if(keyData.key == pgData->keys[currentPosition] && currentPosition < pgData->numberEntries){
        return RC_IM_KEY_ALREADY_EXISTS;
    }
    else{
        updatedKeys[currentPosition] = keyData.key;
        updatedPointers[currentPosition] = keyData.lside;
        updatedPointers[currentPosition+1] = keyData.rside;
        currentPosition++;
        updatedKeys[currentPosition] = pgData->keys[currentPosition-1];
        currentPosition++;
        
    }

    // for (int j = currentPosition; j < page->numberEntries + 2; j++) {
    // updatedKeys[j] = page->keys[j - 1];
    // updatedPointers[j] = page->pointers[j - 1];
    // }
    while(currentPosition<pgData->numberEntries+2){
        updatedKeys[currentPosition]=pgData->keys[currentPosition-1];
        updatedPointers[currentPosition]=pgData->pointers[currentPosition-1];
        currentPosition++;
    }

    updatedPointers[currentPosition] = -1;
    free(pgData->keys);
    free(pgData->pointers);
    pgData->numberEntries++;
    pgData->pointers = updatedPointers;
    pgData->keys = updatedKeys;
   
    return RC_OK;
}

RC keyAndPointerDeletingInLeaf(pgData* pageData, int key){
    int keys[5] = {0};
    float children[5] = {0};
    int i = 0, count = 0;
    bool found = false;
    while (i < pageData->numberEntries) {
        switch (key == pageData->keys[i]) {
            case true:
                if (i < pageData->numberEntries) {
                    found = true;
                    i++; // Skip the current key and move to the next
                    continue;
                }
                break;

            case false:
                keys[count] = pageData->keys[i];
                children[count] = pageData->pointers[i];
                count++;
                break;
        }
        i++;
    }

    if (!found) {
        return RC_IM_KEY_NOT_FOUND; // Key not found, return the appropriate error code
    }

    // Adjust the number of entries and update the last pointer
    pageData->numberEntries--;
    children[count] = -1;
    size_t index = 0;
    while(index < count)
    {
        pageData->keys[index] = keys[index];
        pageData->pointers[index] = children[index];
        index++;
    }
    pageData->pointers[count] = children[count];
    return RC_OK;
}

RC getLeafPg(pgData page,BM_BufferPool* Bmanager,BM_PageHandle* pgHandler,int* lPages){   
    
    if(!page.leaf){
        size_t childIndex = 0;
        while(childIndex<page.numberEntries+1)
        {
            pgData child;
            readPageData(Bmanager,pgHandler,&child,(int)page.pointers[childIndex]);
            if(getLeafPg(child,Bmanager,pgHandler,lPages) == RC_OK){
                childIndex++;
                continue;
            }
            childIndex++;
        }
    }
    else{
        lPages[counter] = page.pgNumber;
        counter += 1;
        return RC_OK;
    }
    return RC_OK;
}

//****************************************************************************************


//********************************Index Acess functions***********************************

// to find the given key
RC findKey(BTreeHandle *tree, Value *key, RID *result){
    
    // Extracting tree data components into local variables
    treeData *treeManager = (treeData *)tree->mgmtData;

    // Loading the main components into local variables
    SM_FileHandle flHandler = treeManager->flHandler;
    BM_PageHandle *pageHander = treeManager->pgHandler;
    BM_BufferPool *Bmanager = treeManager->Bmanager;

    pgData rootPgData;// root page data

    // getting the root page number
    int rootPageNumber=((treeData*)tree->mgmtData)->fMD.rootPageNumber;
    readPageData(Bmanager,pageHander,&rootPgData,rootPageNumber);

    pgData leafPageData= locatePGtoInsert(Bmanager,pageHander,rootPgData,key->v.intV);

    size_t index=0;

    while(index<leafPageData.numberEntries){
        if(leafPageData.keys[index] == key->v.intV){ // checking for the key
            
            float c=leafPageData.pointers[index];
            int cValue =round(c*10);
            int pgNumber= cValue /10;
            int slot=cValue%10;

            // updating slot and page number if key is found
            result->slot=slot;
            result->page=pgNumber;

            return RC_OK;
        }
        index++;
    }

    return RC_IM_KEY_NOT_FOUND;
}

RC insertKey (BTreeHandle *tree, Value *key, RID rid){
    
    //printf("\nstart insert key\n");

    // getting the page handler, buffer manager and file handler
    BM_PageHandle *pgHandler = ((treeData*)tree->mgmtData)->pgHandler;
    BM_BufferPool *Bmanager = ((treeData*)tree->mgmtData)->Bmanager;
    SM_FileHandle flHandler = ((treeData*)tree->mgmtData)->flHandler;

    int maxEntry = ((treeData*)tree->mgmtData)->fMD.maxEntriesPpage;
    int curNumOfNode = ((treeData*)tree->mgmtData)->fMD.numberOfNodes;
    int rootPgNum = ((treeData*)tree->mgmtData)->fMD.rootPageNumber;
    
    pgData rootPage; // getting the root page
    readPageData(Bmanager,pgHandler,&rootPage,rootPgNum);
    //printf("read pg data");

    pgData insertionPage; // getting the page where data can be inserted
    insertionPage = locatePGtoInsert(Bmanager,pgHandler,rootPage,key->v.intV);

    //printf("\nlocated page!------------\n");
    
    if(NewKeyPtr(&insertionPage,key->v.intV,rid) == RC_IM_KEY_ALREADY_EXISTS){
        return RC_IM_KEY_ALREADY_EXISTS;
    }
    
    //printf("\nnew key and ptr to leaf!------------\n");

    if(insertionPage.numberEntries > maxEntry){ // check if there is no space in the leaf node
        
        // creating new nodes
        int counter = 0;
        float *newNodechildren = (float *)malloc(10*sizeof(float));
        int *newNodeKeys = (int *)malloc(10*sizeof(int));
        
        for (size_t i = (int)ceil((insertionPage.numberEntries)/2)+1; i < insertionPage.numberEntries; i++)
        {
            newNodechildren[counter] = insertionPage.pointers[i];
            newNodeKeys[counter] = insertionPage.keys[i];
            counter++;
        }
            // Mark the end of the new node's children
    newNodechildren[counter] = -1;

    // Initialize memory for keys and pointers of the old node
    int *oldNodeKeys = (int *)malloc(10 * sizeof(int));
    float *oldNodeChildren = (float *)malloc(10 * sizeof(float));

    if (oldNodeKeys == NULL || oldNodeChildren == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for old node.\n");
        free(oldNodeKeys);
        free(oldNodeChildren);
        return RC_MEMORY_ALLOCATION_FAILED;
    }

    // Populate keys and pointers for the old node from the insertion page
    int index = 0;
    for (int i = 0; i <= (int)ceil((float)insertionPage.numberEntries / 2); i++) {
        oldNodeKeys[index] = insertionPage.keys[i];
        oldNodeChildren[index] = insertionPage.pointers[i];
        index++;
    }

    // Mark the end of the old node's children
    oldNodeChildren[index] = -1;


        ensureCapacity (curNumOfNode + 2, &flHandler);

        curNumOfNode += 1;

        ((treeData*)tree->mgmtData)->fMD.numberOfNodes++;

        // setting data for rside child
        pgData rsideChild;
        rsideChild.pgNumber = curNumOfNode;
        rsideChild.leaf = 1;
        rsideChild.pointers = newNodechildren;
        rsideChild.numberEntries = (int)floor((maxEntry+1)/2);
        rsideChild.keys = newNodeKeys;

        if(insertionPage.parentnode == -1)rsideChild.parentnode = 3;
        else rsideChild.parentnode = insertionPage.parentnode;

        //printf("start update rside child");
        
        // updating data of rside child
        char *dataHolder;
        aSpace(&dataHolder);
        PageDataToWrite(&rsideChild,dataHolder);
        pWrite(Bmanager,pgHandler,dataHolder,rsideChild.pgNumber);
        dSpace(&dataHolder);
        //printf("updated rside child");


        // lside child data
        pgData lsideChild;
        lsideChild.leaf = 1;
        lsideChild.pgNumber = insertionPage.pgNumber;
        lsideChild.numberEntries = (int)ceil((maxEntry+1)/2)+1;
        lsideChild.keys = oldNodeKeys;
        lsideChild.pointers = oldNodeChildren;
    
        if(insertionPage.parentnode == -1) lsideChild.parentnode = 3;
        else lsideChild.parentnode = insertionPage.parentnode;
        
        //printf("start update lside child");
        
        // updating lside child data
        aSpace(&dataHolder);
        PageDataToWrite(&lsideChild,dataHolder);
        pWrite(Bmanager,pgHandler,dataHolder,lsideChild.pgNumber);
        dSpace(&dataHolder);
        //printf("updated lside child");

        int pgNumber = insertionPage.parentnode;
        float lside = lsideChild.pgNumber, rside = rsideChild.pgNumber;

        data keyData;
        keyData.lside = lside;
        keyData.key = rsideChild.keys[0];
        keyData.rside = rside;

        //printf("progate start");
        propogateUp(tree,pgNumber,keyData); // propagate up
        //printf("progate end");
    }
    else{
        char *dataHolder = malloc(500);
        //printf("at root--------");
        aSpace(&dataHolder);
        PageDataToWrite(&insertionPage,dataHolder);
        pWrite(Bmanager,pgHandler,dataHolder,insertionPage.pgNumber);
        dSpace(&dataHolder);
        //printf("done root");
    }

    ((treeData*)tree->mgmtData)->fMD.nOfEntries++; // change the number of entries

    forceFlushPool(Bmanager); // flush the buffer
    //printf("done key insert");
    return RC_OK;
    
}

// delete key
RC deleteKey (BTreeHandle *tree, Value *key){
    
    // getting buffer manager and page handler
    BM_BufferPool *Bmanager = ((treeData*)tree->mgmtData)->Bmanager;
    BM_PageHandle *pgHandler = ((treeData*)tree->mgmtData)->pgHandler;
    
    pgData rootPg; // getting the root page
    int rootPgIndex = ((treeData*)tree->mgmtData)->fMD.rootPageNumber;
    readPageData(Bmanager,pgHandler,&rootPg,rootPgIndex);

    pgData pageData  = locatePGtoInsert(Bmanager,pgHandler,rootPg,key->v.intV);

    if(keyAndPointerDeletingInLeaf(&pageData,key->v.intV) == RC_IM_KEY_NOT_FOUND) // deleting the key
        return RC_IM_KEY_NOT_FOUND; // if key not found

    char *updatedData;
    
    // updating the data
    aSpace(&updatedData);
    PageDataToWrite(&pageData,updatedData);
    pWrite(Bmanager,pgHandler,updatedData,pageData.pgNumber);
    dSpace(&updatedData);

    return RC_OK;
}

// open tree scan
RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle){
    
    BM_PageHandle *pgHandler = ((treeData*)tree->mgmtData)->pgHandler;

    BM_BufferPool *Bmanager = ((treeData*)tree->mgmtData)->Bmanager;
    
    // allocating space for scan handler and scan manager
    scMgmtData = (scanData*)malloc(sizeof(scanData));
    scanHandle = (BT_ScanHandle*)malloc(sizeof(BT_ScanHandle));
    
    int rootPageNum = ((treeData*)tree->mgmtData)->fMD.rootPageNumber;

    pgData rootPg;
    readPageData(Bmanager,pgHandler,&rootPg,rootPageNum);
    
   // Allocate memory for storing leaf page numbers
    int *leafPages = (int *)malloc(100*sizeof(int));
    counter = 0;
    getLeafPg(rootPg,Bmanager,pgHandler,leafPages);
    //printf("\nInside tree scan!\n");
    scMgmtData->leafPage = leafPages;
    scMgmtData->curPage = leafPages[0];    
    pgData leafPage;
    readPageData(Bmanager,pgHandler,&leafPage,scMgmtData->curPage);
    scMgmtData->curPageData = leafPage;
    scMgmtData->nextPagePosInLeafPages = 1;
    scMgmtData->curPosInPage = 0;
    scMgmtData->noOfLeafPage = counter;
    scMgmtData->isCurPageLoaded = 1;

    scanHandle->mgmtData = scMgmtData;
    scanHandle->tree = tree;
    *handle = scanHandle;

    return RC_OK;
}

//next entry
RC nextEntry (BT_ScanHandle *handle, RID *result){
    
    BM_BufferPool *Bmanager = ((treeData*)handle->tree->mgmtData)->Bmanager;
    BM_PageHandle *pgHandler = ((treeData*)handle->tree->mgmtData)->pgHandler; 
    scanData* scanData = handle->mgmtData;

    if(scanData->curPosInPage >= scanData->curPageData.numberEntries){ // Check if the current position is beyond the number of entries on the current page
        // Check if there are no leaf pages to scan
        if(scanData->nextPagePosInLeafPages == -1 ){
            return RC_IM_NO_MORE_ENTRIES;
        }
        // Move to the next leaf page
        scanData->curPage = scanData->leafPage[scanData->nextPagePosInLeafPages];
        scanData->isCurPageLoaded = 0;
        scanData->nextPagePosInLeafPages += 1;

        if(scanData->nextPagePosInLeafPages >= scanData->noOfLeafPage){
            scanData->nextPagePosInLeafPages = -1;
        }
    }

    if(!scanData->isCurPageLoaded){
        pgData leafPg;
        readPageData(Bmanager,pgHandler,&leafPg,scanData->curPage);
        scanData->curPageData = leafPg;
        scanData->curPosInPage = 0;
        scanData->isCurPageLoaded = 1;   
    }

    // updating slot and page
    float c = scanData->curPageData.pointers[scanData->curPosInPage];
    int cValue = round(c*10);
    result->slot =  cValue % 10;
    result->page =  cValue / 10; 
    scanData->curPosInPage += 1;
    
    return RC_OK;
}

// close tree scan
RC closeTreeScan (BT_ScanHandle *handle){
    free(handle->mgmtData);
    free(handle);
    handle = NULL;
    return RC_OK;
}

//**************************************************************************************************