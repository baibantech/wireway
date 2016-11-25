#ifndef __bptree__
#define __bptree__
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define pointer_size 5
typedef int (*bptree_key_cmp_func)(void *src,void *dst);
typedef unsigned long (*get_data_id_func)(void *data);
typedef unsigned long (*get_key_id_func)(void *data);
typedef unsigned long (*alloc_bptree_node_func)();
typedef struct node_block
{
    unsigned long node_id;
    unsigned long pointers_id[pointer_size];
    unsigned long keys_id[pointer_size -1];
    unsigned long parent_id;
    int num_keys;
    int is_leaf;
}node_block;


typedef struct node {
    void *pointers[pointer_size];
    char *keys[pointer_size-1];
    struct node *parent;
    int num_keys;
    bool is_leaf;
    node_block *block;
    struct tree_root *tree_root; 
} node;

typedef struct tree_root {
    int save;

    bptree_key_cmp_func  key_cmp;
    get_data_id_func     get_data_id;
    get_key_id_func      get_key_id;
    alloc_bptree_node_func alloc_block_func;
 
    node *node_root;
}tree_root;

#if 1 
typedef struct queue {
    int capacity;
    int front;
    int rear;
    int size;
    node **items;
} queue;
#endif


void print_tree(node *root);
void *find(node *root, char *key);
node *find_leaf(node *root, char *key);

// Insertion
node *make_new_node(tree_root *tree);
node *make_new_leaf(tree_root *tree);
node *make_new_tree(tree_root *tree,char *key, void *data);
node *make_new_root(node *left, node *right, char *key,unsigned long key_id);
node *insert(node *root, char *key,void *data);
node *insert_into_parent(node *root, node *left, node *right, char *key,unsigned long key_id);
void insert_into_node(node *nd, node *right, int index, char *key,unsigned long key_id);
node *insert_into_node_after_splitting(node *root, node *nd, node *right,
                                       int index, char *key,unsigned long key_id);
node *insert_into_leaf_after_splitting(node *root, node *leaf, int index,
                                       char *key, void *data);
void insert_into_leaf(node *leaf, int index, char *key, void *data);

// Deletion
void destroy_node(node *nd);
void destroy_tree(node *root);
void *remove_entry(node *nd, int index);
node *delete(node *root, char *key);
node *delete_entry(node *root, node *nd, int index);
node *adjust_root(node *root);
int get_node_index(node *nd);
node *coalesce_nodes(node *root, node *nd, node *neighbor, int nd_index);
void distribute_nodes(node *nd, node *neighbor, int nd_index);
node *restore_bptree_root();
#endif
