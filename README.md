# File-System 

## Introduction
This repository contains the implementation of a simulated file system (simfs) in C. The simulated file system is stored in a single Unix file, featuring a single directory with limits on the number of files it can store (MAXFILES) and the amount of space it can occupy (MAXBLOCKS). Your task is to implement functions for creating a file, deleting a file, writing to a file, and reading from a file.

## simfs Structure
The simfs is structured as an array of blocks, each a contiguous chunk of BLOCKSIZE bytes. Metadata, including file entries (fentries) and file nodes (fnodes), is stored at the beginning of the file. The structure of fentries and fnodes is crucial for file system operations, so careful adherence to the specifications is essential.

## Starter Code
The repository includes starter code that you should build upon. Note that you are not allowed to modify the structs defined in simfstypes.h, and you should not commit any changes to it. The Makefile is structured for efficient compilation, and you may need to modify the switch statement in simfs.c as you add new commands.

## Features Implentated

1. **Creating a file (createfile):**
   - Takes a simulated file name.
   - Creates an empty file if possible, using the first available fentry.
   - Emits an error if resources are insufficient.

2. **Deleting a file (deletefile):**
   - Takes a simulated file name.
   - Removes the file from the file system, freeing associated blocks.
   - Overwrites file data with zeroes for security.

3. **Reading a file (readfile):**
   - Takes file name, start offset, and length.
   - Prints requested data to stdout.
   - Emits an error if the request cannot be completed.

4. **Writing a file (writefile):**
   - Takes file name, start offset, and length.
   - Writes data from stdin to the file.
   - Emits an error if any part of the request cannot be completed.

5. **Fingerprinting a file (info):**
   - Takes a file name.
   - Prints data to stdout, counting the most common character.
   - Emits an error if the request cannot be completed.

## General Guidelines
- Your implementation should handle any valid values of BLOCKSIZE, MAXFILES, or MAXBLOCKS.
- Avoid leaving the file system in an inconsistent state; the operation must be completed entirely or not at all.
- Initialize new data blocks to zeros for debugging assistance.

## Error Handling
- Thorough error checking is required to prevent crashes and ensure file system consistency.
- Print appropriate error messages to stderr, describing the encountered issues.
- No non-error messages should be printed to stderr.

## Usage
1. **Build the Program:**
   ```bash
   make
   ```

2. **Initialize File System:**
   ```bash
   ./simfs initfs filename
   ```

3. **Run Commands:**
   ```bash
   ./simfs createfile filename
   ./simfs deletefile filename
   ./simfs readfile filename start length
   ./simfs writefile filename start length
   ./simfs info filename
   ```

4. **Print File System:**
   ```bash
   ./simfs printfs filename
   ```
