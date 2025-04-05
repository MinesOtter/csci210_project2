#include "types.h"
#include <string.h>
#include <stdlib.h>

extern struct NODE* root;
extern struct NODE* cwd;

//make directory
void mkdir(char pathName[]){
    char baseName[64];
    char dirName[256];
    
    // Check if path is provided
    if (strcmp(pathName, "/") == 0) {
        printf("MKDIR ERROR: no path provided\n");
        return;
    }
    
    // Get parent directory node and split the path
    struct NODE* parentDir = splitPath(pathName, baseName, dirName);
    
    // If splitPath returned NULL, there was an error finding the parent directory
    if (parentDir == NULL) {
        return;
    }
    
    // Check if directory already exists
    struct NODE* child = parentDir->childPtr;
    while (child != NULL) {
        if (strcmp(child->name, baseName) == 0 && child->fileType == 'D') {
            // Use the full pathName in the error message instead of just baseName
            printf("MKDIR ERROR: directory %s already exists\n", pathName);
            return;
        }
        child = child->siblingPtr;
    }
    
    // Create new directory node
    struct NODE* newDir = (struct NODE*)malloc(sizeof(struct NODE));
    if (newDir == NULL) {
        printf("MKDIR ERROR: memory allocation failed\n");
        return;
    }
    
    // Initialize the new directory node
    strncpy(newDir->name, baseName, 63);
    newDir->name[63] = '\0';
    newDir->fileType = 'D';
    newDir->childPtr = NULL;
    newDir->siblingPtr = NULL;
    newDir->parentPtr = parentDir;
    
    // Add the new directory to the parent's children
    if (parentDir->childPtr == NULL) {
        // First child
        parentDir->childPtr = newDir;
    } else {
        // Add as sibling to the last child
        child = parentDir->childPtr;
        while (child->siblingPtr != NULL) {
            child = child->siblingPtr;
        }
        child->siblingPtr = newDir;
    }
    
    printf("MKDIR SUCCESS: node %s successfully created\n", pathName);
    return;
}

//handles tokenizing and absolute/relative pathing options
struct NODE* splitPath(char* pathName, char* baseName, char* dirName){
    // Initialize baseName and dirName as empty strings
    baseName[0] = '\0';
    dirName[0] = '\0';
    
    // Handle special case for root directory
    if (strcmp(pathName, "/") == 0) {
        strcpy(dirName, "/");
        baseName[0] = '\0';
        return root;
    }
    
    // Make a copy of pathName to avoid modifying the original
    char pathCopy[256];
    strncpy(pathCopy, pathName, 255);
    pathCopy[255] = '\0';
    
    // Find the last '/' in the path
    char* lastSlash = strrchr(pathCopy, '/');
    
    if (lastSlash == NULL) {
        // No '/' in the path, so it's a file/directory in the current directory
        strcpy(baseName, pathCopy);
        dirName[0] = '\0';
        return cwd;
    } else {
        // Extract baseName (everything after the last '/')
        strcpy(baseName, lastSlash + 1);
        
        // Extract dirName (everything up to and including the last '/')
        if (lastSlash == pathCopy) {
            // Path starts with '/' and has no other '/'
            strcpy(dirName, "/");
        } else {
            // Copy everything up to the last '/'
            *lastSlash = '\0';
            strcpy(dirName, pathCopy);
        }
    }
    
    // Determine the starting node based on whether the path is absolute or relative
    struct NODE* currentNode;
    char* token;
    char dirCopy[256];
    
    if (dirName[0] == '/') {
        // Absolute path
        currentNode = root;
        
        // Skip the leading '/' for tokenization
        if (strlen(dirName) > 1) {
            strcpy(dirCopy, dirName + 1);
            token = strtok(dirCopy, "/");
            
            // Traverse the path
            while (token != NULL) {
                struct NODE* found = NULL;
                struct NODE* child = currentNode->childPtr;
                
                // Look for the directory in the current node's children
                while (child != NULL) {
                    if (strcmp(child->name, token) == 0 && child->fileType == 'D') {
                        found = child;
                        break;
                    }
                    child = child->siblingPtr;
                }
                
                if (found == NULL) {
                    // Directory not found
                    printf("ERROR: directory %s does not exist\n", token);
                    return NULL;
                }
                
                currentNode = found;
                token = strtok(NULL, "/");
            }
        }
    } else {
        // Relative path
        currentNode = cwd;
        
        if (strlen(dirName) > 0) {
            strcpy(dirCopy, dirName);
            token = strtok(dirCopy, "/");
            
            // Traverse the path
            while (token != NULL) {
                struct NODE* found = NULL;
                struct NODE* child = currentNode->childPtr;
                
                // Look for the directory in the current node's children
                while (child != NULL) {
                    if (strcmp(child->name, token) == 0 && child->fileType == 'D') {
                        found = child;
                        break;
                    }
                    child = child->siblingPtr;
                }
                
                if (found == NULL) {
                    // Directory not found
                    printf("ERROR: directory %s does not exist\n", token);
                    return NULL;
                }
                
                currentNode = found;
                token = strtok(NULL, "/");
            }
        }
    }
    
    return currentNode;
}
