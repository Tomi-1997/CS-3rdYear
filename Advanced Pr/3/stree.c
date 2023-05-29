/* 
Stree- uses nftw() to build a tree of the current file tree,
then prints information about each file.
Might be inefficient as it passes the whole file tree twice.
*/

#define _XOPEN_SOURCE 600 /* Get nftw() */
#include <ftw.h>
#include <sys/types.h>    /* Type definitions used by many programs */
#include <stdio.h>        /* Standard I/O functions */
#include <stdlib.h>       /* Prototypes of commonly used library functions,
                             plus EXIT_SUCCESS and EXIT_FAILURE constants */
#include <unistd.h>       /* Prototypes for many system calls */
#include <errno.h>        /* Declares errno and defines error constants */
#include <string.h>       /* Commonly used string-handling functions */


#define CHILD_MAX 16
#define PATH_MAX 128
#define PREFIX_MAX 128

typedef struct Node_
{
    int level;
    char pathname[PATH_MAX];           /* Could be malloc'd instead */
    struct Node_* children[CHILD_MAX]; /* Could be turned into a linked list */
    char prefix[PREFIX_MAX];
    int children_size;
    struct Node_* prev;
} Node;


typedef struct Tree_
{
    Node* root;
    int size;
} Tree;

int treeify(const char* pathname, const struct stat* sbuf, int type, struct FTW* ftwb);
void climb_(Node* root);
void climb(Node* curr, int last);

char* LAST_SIGN = "└── ";
char* MID_SIGN = "├── ";

size_t DIRS = 0;
size_t FILES = 0;
Tree tree;
int main(int argc, char* argv[])
{
    /* Tree init */
    tree.root = NULL;
    tree.size = 0;
    /*************/

    int flags = 0;
    if (argc > 2) 
    {
        fprintf(stderr, "Usage: ./%s directory-path\n pr ./%s", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Is there a no argument? then take current folder as the argument */
    char* arg = (argc == 1) ? "." : argv[1]; 


    if (nftw(arg, treeify, 10, flags) == -1) 
    {
        perror("ftw");
        exit(EXIT_FAILURE);
    }

    climb_(tree.root);
    return EXIT_SUCCESS;
}


void climb_(Node* root)
{
    printf("%s\n", root->pathname);
    for (int i = 0; i < root->children_size; i++)
    {
        int is_last = (i == root->children_size -1);
        // strcpy(root->children[i]->prefix, "│   ");
        climb(root->children[i], is_last? 1 : 0);
    }
}


char* path_to_name(char* path)
{
    int i = strlen(path) - 1;
    while (path[i] != '/')
    {
        i--;
    }
    return path + i + 1;
}


void climb(Node* curr, int last)
{
    if (curr == NULL)
        return;

    printf("%s", curr->prev == tree.root? "" : curr->prefix);
    printf("%s", last? LAST_SIGN : MID_SIGN); 
    printf(" %s\n", path_to_name(curr->pathname));

    int len = curr->children_size;
    int last_entry = (curr->prev == tree.root && last == 1);

    char* sign = "    ";
    if (len > 0 && !last_entry && last == 0)
    {
        sign = "│   ";
    }
    strcat(curr->prefix, sign);
    for (int i = 0; i < len; i++)
    {      
        int is_last = (i == curr->children_size -1);
        strcat(curr->children[i]->prefix, curr->prefix);
        climb(curr->children[i], is_last? 1 : 0);
        
    }
}


Node* from_path(const char* pathname, Node* curr)
{    
    if (curr == NULL)
        return NULL;

    if (strcmp( curr->pathname, pathname ) == 0)
        return curr;

    for (int i = 0; i < curr->children_size; i++)
    {
        Node* res = from_path(pathname, curr->children[i]);
        if (res != NULL)
            return res;
    }
    return NULL;
}



Node* get_parent(const char* pathname)
{
    char parent_path[PATH_MAX];
    int end = strlen(pathname) - 1;
    
    /* Extract parent path, find where the child's name starts and remove it */
    while (pathname[end] != '/')
    {
        end--;
    }
    int i = 0;
    for (; i < end; i++)
        parent_path[i] = pathname[i];
    parent_path[i] = '\0';
    /* Find parent by pathname */
    return from_path(parent_path, tree.root);
}


int treeify(const char* pathname, const struct stat* sbuf, int type, struct FTW* ftwb)
{    
    /* Initialize a new node with NULL children */
    Node* current = (Node*) (malloc(sizeof(Node)));
    strcpy(current->pathname, pathname);
    current->level = ftwb->level;
    for (int i = 0; i < CHILD_MAX; i++)
        current->children[i] = NULL;

    /* Root */
    if (ftwb->level == 0)
    {
        current->prev = NULL;
        tree.root = current;
    }
    /* Not root, find parent by complete pathname and add new node to parent's child list.*/
    else
    {
        Node* parent = get_parent(pathname);
        current->prev = parent;
        if (parent->children_size == CHILD_MAX - 1)
        {
            puts("Child max");
            return EXIT_FAILURE;
        }
        int csize = parent->children_size;
        parent->children[csize] = current;
        parent->children_size = csize + 1;
    }
    tree.size++;
    return 0;
}
