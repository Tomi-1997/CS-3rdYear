/* 
Stree- uses nftw() to build a tree of the current file tree,
then prints information about each file.
Not that efficient as it passes the whole file tree twice.
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
#include <pwd.h>
#include <grp.h>

#define CHILD_MAX 16
#define PATH_MAX 128
#define PREFIX_MAX 128

typedef struct Node_
{
    int level;
    int children_size;
    struct Node_* prev;
    char prefix[PREFIX_MAX];
    char pathname[PATH_MAX];           /* Could be malloc'd instead */
    struct Node_* children[CHILD_MAX]; /* Could be turned into a linked list */
} Node;


typedef struct Tree_
{
    Node* root;
    int size;
} Tree;

/* Use ftw to build tree */
int treeify(const char* pathname, const struct stat* sbuf, int type, struct FTW* ftwb);
void climb_(Node* root);            /* Print */
void climb(Node* curr, int last);
void chop(Node* root);              /* Delete */
void print_stats(Node* file);

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
        fprintf(stderr, "Usage: %s directory-path, for example:\n%s ..\n%s\n", argv[0], argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    /* No arg - take current folder */
    char* arg = (argc == 1) ? "." : argv[1]; 

    /* Pass all files and create tree */
    if (nftw(arg, treeify, 10, flags) == -1) 
    {
        perror("ftw");
        exit(EXIT_FAILURE);
    }

    /* Print tree */
    climb_(tree.root);

    /* Delete tree */
    chop(tree.root);
    
    /* DIRS - 1: Subtract one unless you want to count root folder in the counter */
    printf("%ld directories, %ld files\n", (DIRS - 1), FILES);
    return EXIT_SUCCESS;
}


void chop(Node* root)
{
    if (root == NULL)
        return;

    for (int i = 0; i < root->children_size; i++)
    {
        chop(root->children[i]);
    }
    free(root);
}


void print_stats(Node* file)
{
    struct stat st;
    if (stat(file->pathname, &st) != 0) 
    {
        perror(file->pathname);
        exit(1);
    }

    char permissions[10];

    char file_type = '-';
    /* File type (directory, socket, etc) */
    switch (st.st_mode & S_IFMT) 
    {
        case S_IFREG:  file_type = '-'; break;
        case S_IFDIR:  file_type = 'd'; break;
        case S_IFCHR:  file_type = 'c'; break;
        case S_IFBLK:  file_type = 'b'; break;
        case S_IFLNK:  file_type = 'l'; break;
        case S_IFIFO:  file_type = 'p'; break;
        case S_IFSOCK: file_type = 's'; break;
        default:       file_type = '?'; break; /* Should never happen (on Linux) */
    }

    permissions[0] = file_type;

    /* User/Owner permissions */
    permissions[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';

    /* Group permissions */
    permissions[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';

    /* Not owner or group, other*/
    permissions[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';

    /* User name*/
    struct passwd* pw = getpwuid(st.st_uid);
    /* Group name*/
    struct group* gr = getgrgid(st.st_gid);

    printf(" [%s %s  %s        %ld]", permissions,  gr->gr_name, pw->pw_name, st.st_size);
}


void climb_(Node* root)
{
    if (root == NULL)
        return;
    printf("%s\n", root->pathname);
    for (int i = 0; i < root->children_size; i++)
    {
        int is_last = (i == root->children_size -1);
        climb(root->children[i], is_last? 1 : 0);
    }
}

/* Returns only the name from the full path
   Input: /user/program/file -> file
   */
char* path_to_name(char* path)
{
    int i = strlen(path) - 1;
    while (path[i] != '/' && i >= 0)
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

    print_stats(curr); // [permissions group user size]
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


/* Returns Node* object from tree by given full path */
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
    /* Increment folder or directory counter */
    if (S_ISDIR(sbuf->st_mode))
        DIRS++;
    else
        FILES++;

    /* Initialize a new node with NULL children */
    Node* current = (Node*) (malloc(sizeof(Node)));
    if (current == NULL)
    {
        printf("Malloc() error at treeify");
        return EXIT_FAILURE;
    }

    /* Initialize node's variables */
    current->prev = NULL;
    current->children_size = 0;
    current->level = ftwb->level;
    strcpy(current->pathname, pathname);
    memset(current->prefix, '\0', sizeof(current->prefix));
    for (int i = 0; i < CHILD_MAX; i++)
        current->children[i] = NULL;


    /* Root */
    if (ftwb->level == 0)
    {
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
