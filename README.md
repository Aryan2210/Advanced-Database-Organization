# Advanced Database Organization - CS525 Assignments

This repository contains programming assignments for the **CS525 Advanced Database Organization** course. These assignments explore critical aspects of database system design, implementation, and optimization.

---

## 📂 Assignments Overview

### Assignment 1: Basic Storage Manager
- **Description**: Implemented a basic storage manager to interact with page files and handle low-level data storage.
- **Key Features**:
  - Page file creation, reading, and writing.
  - Error handling mechanisms for file operations.
- **Files**:
  - `storage_mgr.c`, `storage_mgr.h`
  - `dberror.c`, `dberror.h`
  - `test_assign1_1.c`

#### Key Functions:
| Function Name       | Description                                                                                   |
|---------------------|-----------------------------------------------------------------------------------------------|
| `createPageFile`    | Creates a new page file on disk.                                                              |
| `openPageFile`      | Opens an existing page file.                                                                  |
| `closePageFile`     | Closes an open page file.                                                                     |
| `destroyPageFile`   | Deletes a page file from disk.                                                                |
| `readBlock`         | Reads a specific block from a page file into memory.                                          |
| `writeBlock`        | Writes a block of data to a specific position in a page file.                                 |
| `appendEmptyBlock`  | Adds a new empty block to the end of a page file.                                             |
| `ensureCapacity`    | Ensures a page file has a minimum number of blocks by appending empty blocks if necessary.    |

---

### Assignment 2: Buffer Manager
- **Description**: Developed a buffer manager to optimize memory utilization and enable efficient page replacement, building on Assignment 1.
- **Key Features**:
  - Buffer pool initialization and shutdown.
  - Support for page replacement strategies like FIFO, LRU, and CLOCK.
  - Functions for marking dirty pages, pinning/unpinning pages, and flushing buffers.
- **Files**:
  - `buffer_mgr.c`, `buffer_mgr.h`
  - `buffer_mgr_stat.c`, `buffer_mgr_stat.h`
  - `storage_mgr.c`, `storage_mgr.h`
  - `test_assign2_1.c`, `test_assign2_2.c`

#### Key Functions:
| Function Name       | Description                                                                                   |
|---------------------|-----------------------------------------------------------------------------------------------|
| `initBufferPool`    | Initializes a buffer pool.                                                                    |
| `shutdownBufferPool`| Releases all resources associated with the buffer pool.                                       |
| `forceFlushPool`    | Writes all dirty pages in the buffer pool back to disk.                                       |
| `markDirty`         | Marks a specified page as dirty.                                                              |
| `unpinPage`         | Decreases the fix count of a page.                                                            |
| `forcePage`         | Immediately writes a specific page back to disk.                                              |
| `pinPageFIFO`       | Implements the FIFO page replacement strategy.                                                |
| `pinPageLRU`        | Implements the LRU page replacement strategy.                                                 |
| `pinPageCLOCK`      | Implements the CLOCK page replacement strategy.                                               |

---

### Assignment 3: Buffer Manager
- **Description**: Extended the buffer manager with advanced functionalities and performance optimization.
- **Key Features**:
  - Enhanced memory management using improved page replacement policies.
  - Integrated buffer pool diagnostics for efficient troubleshooting.
- **Files**:
  - `buffer_mgr.c`, `buffer_mgr.h`
  - `buffer_mgr_stat.c`, `buffer_mgr_stat.h`
  - `storage_mgr.c`, `storage_mgr.h`
  - `test_assign3_1.c`, `test_expr.c`

#### Key Functions:
| Function Name           | Description                                                                       |
|-------------------------|-----------------------------------------------------------------------------------|
| `getNumReadIO`          | Retrieves the number of pages read from disk by the buffer manager.               |
| `getNumWriteIO`         | Retrieves the number of pages written to disk by the buffer manager.              |
| `getBufferDiagnostics`  | Provides detailed statistics for debugging buffer pool behavior.                  |

---

### Assignment 4: B+ Tree Indexing
- **Description**: Developed a B+ Tree-based indexing system to enhance query performance.
- **Key Features**:
  - Insert, delete, and search operations in B+ Trees.
  - Integration with the buffer manager for efficient data access.
- **Files**:
  - `btree_mgr.c`, `btree_mgr.h`
  - `buffer_mgr.c`, `buffer_mgr.h`
  - `test_assign4_1.c`, `test_expr.c`

#### Key Functions:
| Function Name       | Description                                                                           |
|---------------------|---------------------------------------------------------------------------------------|
| `insertKey`         | Inserts a key-value pair into the B+ Tree.                                            |
| `deleteKey`         | Removes a key from the B+ Tree.                                                       |
| `searchKey`         | Searches for a key in the B+ Tree and returns its associated value.                   |
| `scanNext`          | Iterates through keys in sorted order using a scan operation.                         |

---

## 🚀 How to Build and Run
1. Clone this repository:
   ```bash
   git clone https://github.com/your-username/CS525-Advanced-Database-Organization.git
   cd CS525-Advanced-Database-Organization
   ```
2. Navigate to the assignment directory (e.g., `assign_1`):
   ```bash
   cd assign_1
   ```
3. Use the `Makefile` to compile:
   ```bash
   make
   ```
4. Run the test cases:
   ```bash
   ./test_assign1
   ```

---

## 🔧 Tools and Technologies
- Programming Language: C
- Build System: `Makefile`

---

## 🤝 Contributions
Contributions are welcome! Feel free to fork this repository and submit pull requests to enhance the implementations or add new features.
