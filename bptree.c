#include "bptree.h"
#include "save_func.h"
#define MAX_KEY_LEN 30
#define MAX_NUM_VALUE 50

// each key is a string pointer
// num_keys if the number of keys, number of pointers = num_keys + 1

int size = 5;  // number of pointers of each node

// Bulk Loading -- build B+-tree after sorting
node *bulk_load(char **keys, int *values, int n);
int cmp(const void *, const void *);
queue *init_queue(int capacity)
{
    queue *q;
    q = malloc(sizeof(queue));
    q->items = malloc(sizeof(node *) * capacity);
    q->capacity = capacity;
    q->front = q->rear = 0;
    q->size = 0;
    return q;
}

void enqueue(queue *q, node *nd)
{
    if (q->size == q->capacity) {
        fprintf(stderr, "the queue is full !!\n");
        return ;
    }
    q->items[q->rear] = nd;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

node *dequeue(queue *q)
{
    node *nd;
    if (q->size == 0) {
        fprintf(stderr, "the queue is empty !!\n");
        return NULL;
    }
    nd = q->items[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return nd;
}

int get_level(node *root, node *nd)
{
    int level = 0;
    while (nd != root) {
        nd = nd->parent;
        level++;
    }
    return level;
}

void print_tree(node *root)
{
    queue *q;
    node *nd;
    int level, new_level, i;
    if (root == NULL) {
        printf("Empty tree !\n");
        return ;
    }
    q = init_queue(MAX_NUM_VALUE);
    enqueue(q, root);
    level = 0;
    while (q->size > 0) {
        nd = dequeue(q);
        new_level = get_level(root, nd);
        if (new_level > level) {
            printf("\n");
            level = new_level;
        }
        for (i = 0; i < nd->num_keys; i++)
            printf("%s ", nd->keys[i]);
        printf("| ");
        if (!nd->is_leaf) {
            for (i = 0; i <= nd->num_keys; i++)
                enqueue(q, nd->pointers[i]);
        }
    }
    printf("\n");
    free(q->items);
    free(q);
}

node *load_bptree_node(unsigned long node_id)
{
     int i;
     node *nd;    
     node_block  *nd_block = (node*)read_data(node_id);
    
     if(nd_block)
     {   
        nd = malloc(sizeof(node));
        if(nd)
        {
            nd->block = nd_block;
            nd->num_keys = nd_block->num_keys;
            nd->is_leaf = nd_block->is_leaf;
            for(i = 0; i < nd->num_keys ; i++)
            {
                nd->keys[i] = read_data(nd_block->keys_id[i]);
                if(NULL == nd->keys[i])
                {
                    destroy_node(nd);
                    return NULL;
                }
            }
            for(i = 0; i < size ; i++)
            {
                nd->pointers[i] = NULL;
            }             
            return nd;
        }
        free(nd_block);   
     } 
   
     return NULL;
}

node *restore_bptree_root()
{
    unsigned long node_id = get_bptree_rootid();
    if(node_id != -1)
    {
        return load_bptree_node(node_id);
    }
    return NULL;
}

void *find(node *root, char *key)
{
    node *leaf;
    int i;
    leaf = find_leaf(root, key);
    if (leaf == NULL)
        return NULL;
    for (i = 0; i < leaf->num_keys && strcmp(leaf->keys[i], key) != 0; i++)
        ;
    if (i == leaf->num_keys)
        return NULL;
    return leaf->pointers[i];
}

int is_key_exist(node *root,char *key)
{
    node *leaf;
    int i;
    leaf = find_leaf(root, key);
    if (leaf == NULL)
        return 0;
    for (i = 0; i < leaf->num_keys && strcmp(leaf->keys[i], key) != 0; i++)
        ;
    if (i == leaf->num_keys)
        return 0;
    return 1;
}

node *find_leaf(node *root, char *key)
{
    node *nd;
    int i;
    if (root == NULL)
        return root;
    nd = root;
    while (!nd->is_leaf){
        for (i = 0; i < nd->num_keys && strcmp(nd->keys[i], key) <= 0; i++)
            ;
        if(NULL == nd->pointers[i])
        {
            nd->pointers[i] = load_bptree_node(nd->block->pointers_id[i]);
            nd = nd->pointers[i];
        }
    }
    return nd;
}

node *make_new_node()
{
    node *nd;
    node_block *block; 
    
    nd = (node *)malloc(sizeof(node));
    if(NULL == nd)
    {
        return NULL;
    }
    nd->parent = NULL;
    nd->num_keys = 0;
    nd->is_leaf = false;

    block = (node_block*)malloc(sizeof(node_block));
    
    if(NULL == block)
    {
        free(nd);
        return NULL;
    }
        
    memset(block,0 ,sizeof(node_block));
    block->node_id = alloc_bptree_block();
    if(block->node_id == -1)
    {
        free(nd);
        free(block);
        return NULL;
    }
    
    nd->block = block;
    return nd;
}

node *make_new_leaf()
{
    node *leaf = NULL;
    leaf = make_new_node();
    if(leaf)
    leaf->is_leaf = true;
    return leaf;
}

void destroy_node(node *nd)
{
     free(nd->block);
     free(nd);
}

node *make_new_tree(char *key, void *data)
{
    node *root;
    node_block *block;
    unsigned long key_id;
    unsigned long data_id;

    root = make_new_leaf();
    if(NULL == root)
    {
        return NULL;
    }

    root->pointers[0] = data;
    root->keys[0] = malloc(MAX_KEY_LEN);
    strcpy(root->keys[0], key);
    root->pointers[size-1] = NULL;
    root->num_keys++;
    key_id = get_bptree_keyid(data);
    printf("key id size is %d\r\n",sizeof(key_id));
    printf("key id is 0x%llx\r\n",key_id);
    data_id = get_bptree_dataid(data);
    if(key_id != -1 && data_id != -1)
    {
        block = root->block;
        block->pointers_id[0] = data_id;
        block->keys_id[0] = key_id;
        block->num_keys = root->num_keys;
        block->is_leaf = root->is_leaf;
        save_bptree_node(block,1);
    }
    else
    {
        free_data_block(data_id);
        free_key_block(key_id);
        return NULL;
    }

    return root;
}

void destroy_tree(node *root)
{
    int i;
    if (!root->is_leaf) {
        for (i = 0; i < root->num_keys; i++) {
            free(root->keys[i]);
            destroy_tree(root->pointers[i]);
        }
        destroy_tree(root->pointers[i]);
    }
    else {
        for (i = 0; i < root->num_keys; i++) {
            free(root->keys[i]);
            free(root->pointers[i]); // free record
        }
    }
    destroy_node(root);
}

node *make_new_root(node *left, node *right, char *key,unsigned long key_id)
{
    node *root;
    root = make_new_node();
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->keys[0] = malloc(MAX_KEY_LEN);
    strcpy(root->keys[0], key);
    root->num_keys++;
    left->parent = root;
    right->parent = root;

    root->block->pointers_id[0] = left->block->node_id;
    root->block->pointers_id[1] = right->block->node_id;
    root->block->keys_id[0] = key_id;
    root->block->num_keys = root->num_keys;
    left->block->parent_id = root->block->node_id;
    right->block->parent_id = root->block->node_id;

    return root;
}

node *insert(node *root, char *key, void *data)
{
    node *leaf;
    int index, cond;
    leaf = find_leaf(root, key);
    if (!leaf){  // cannot find the leaf, the tree is empty
        return make_new_tree(key, data);
    }
    for (index = 0; index < leaf->num_keys && (cond = strcmp(leaf->keys[index], key)) < 0; index++)
        ;
    if (cond == 0)  // ignore duplicates
        return root;
    if (leaf->num_keys < size - 1){
        insert_into_leaf(leaf, index, key, data);
        return root;  // the root remains unchanged
    }
    return insert_into_leaf_after_splitting(root, leaf, index, key, data);
}

node *insert_into_parent(node *root, node *left, node *right, char *key,unsigned long key_id)
{
    node *parent;
    int index, i;
    parent = left->parent;

    if (parent == NULL){
        return make_new_root(left, right, key,key_id);
    }

    for (index = 0; index < parent->num_keys && parent->pointers[index] != left; index++);
        ;
    if (parent->num_keys < size - 1){
        insert_into_node(parent, right, index, key,key_id);
        return root;  // the root remains unchanged
    }
    return insert_into_node_after_splitting(root, parent, right, index, key,key_id);
}

void insert_into_node(node *nd, node *right, int index, char *key,unsigned long key_id)
{
    int i;
    for (i = nd->num_keys; i > index; i--){
        nd->keys[i] = nd->keys[i-1];
        nd->pointers[i+1] = nd->pointers[i];
        
        nd->block->keys_id[i] = nd->block->keys_id[i-1];
        nd->block->pointers_id[i] = nd->block->pointers_id[i-1];
    }
    
    nd->keys[index] = malloc(MAX_KEY_LEN);
    strcpy(nd->keys[index], key);
    nd->pointers[index+1] = right;
    nd->num_keys++;

    nd->block->keys_id[index] = key_id;
    nd->block->pointers_id[index+1] = right->block->node_id;
    nd->block->num_keys = nd->num_keys;
}

node *insert_into_node_after_splitting(node *root, node *nd, node *right, int index, char *key,unsigned long key_id)
{
    int i, split;
    node **temp_ps, *new_nd, *child;
    char **temp_ks, *new_key;
    unsigned long *temp_ps_block,*temp_ks_block;
    unsigned long new_key_id; 
    
    temp_ps = malloc((size + 1) * sizeof(node *));
    temp_ks = malloc(size * sizeof(char *));
    
    temp_ps_block = malloc((size+1)*sizeof(unsigned long));
    temp_ks_block = malloc(size*sizeof(unsigned long));

    for (i = 0; i < size + 1; i++){
        if (i == index + 1){
            temp_ps[i] = right;
            temp_ps_block[i] = right->block->node_id; 
        }
        else if (i < index + 1){
            temp_ps[i] = nd->pointers[i];
            temp_ps_block[i] = nd->block->pointers_id[i];
        }
        else {    
            temp_ps[i] = nd->pointers[i-1];
            temp_ps_block[i] = nd->block->pointers_id[i-1];
        }
    }
  #if 1 
    for (i = 0; i < size; i++){
        if (i == index){
            temp_ks[i] = malloc(MAX_KEY_LEN);
            strcpy(temp_ks[i], key);
            temp_ks_block[i] = key_id;
        }
        else if (i < index){
            temp_ks[i] = nd->keys[i];
            temp_ks_block[i] = nd->block->keys_id[i];
        }
        else {
            temp_ks[i] = nd->keys[i-1];
            temp_ks_block[i] = nd->block->keys_id[i-1];
        }
    }
#endif
    split = size % 2 ? size / 2 + 1 : size / 2;  // split is #pointers
    nd->num_keys = split - 1;
    nd->block->num_keys = split-1; 
    for (i = 0; i < split - 1; i++){
        nd->pointers[i] = temp_ps[i];
        nd->keys[i] = temp_ks[i];
        nd->block->pointers_id[i] =  temp_ps_block[i];
        nd->block->keys_id[i] = temp_ks_block[i];
    }
    nd->pointers[i] = temp_ps[i];  // i == split - 1
    nd->block->pointers_id[i] = temp_ps_block[i];
    new_key = temp_ks[split - 1];
    new_key_id = temp_ks_block[split - 1];    


    new_nd = make_new_node();
    new_nd->num_keys = size - split;
    new_nd->block->num_keys= size - split;

    for (++i; i < size; i++){
        new_nd->pointers[i - split] = temp_ps[i];
        new_nd->keys[i - split] = temp_ks[i];
        new_nd->block->pointers_id[i - split] = temp_ps_block[i];
        new_nd->block->keys_id[i - split] = temp_ks_block[i];
    
    }
    new_nd->pointers[i - split] = temp_ps[i];
    new_nd->block->pointers_id[i - split] = temp_ps_block[i];
    new_nd->parent = nd->parent;
    new_nd->block->parent_id = nd->parent->block->node_id;

    for (i = 0; i <= new_nd->num_keys; i++){  //  #pointers == num_keys + 1
        child = (node *)(new_nd->pointers[i]);
        child->parent = new_nd;
        child->block->parent_id = new_nd->block->node_id;
    }

    free(temp_ps);
    free(temp_ks);
    free(temp_ps_block);
    free(temp_ks_block);

    return insert_into_parent(root, nd, new_nd, new_key,new_key_id);
}

void insert_into_leaf(node *leaf, int index, char *key, void *data)
{
    int i;
    unsigned long key_id;
    unsigned long data_id;
    for (i = leaf->num_keys; i > index; i--){
        leaf->keys[i] = leaf->keys[i-1];
        leaf->pointers[i] = leaf->pointers[i-1];

        leaf->block->keys_id[i] = leaf->block->keys_id[i-1];
        leaf->block->pointers_id[i] = leaf->block->keys_id[i-1];
        
    }
    leaf->keys[index] = malloc(MAX_KEY_LEN);
    strcpy(leaf->keys[index], key);
    leaf->pointers[index] = data;
    leaf->num_keys++;
    
    key_id = get_bptree_keyid(data);
    data_id = get_bptree_dataid(data);
    
    if(key_id != -1 && data_id != -1)
    {
        leaf->block->pointers_id[index] = data_id;
        leaf->block->keys_id[index] = key_id;
        leaf->block->num_keys = leaf->num_keys;
        save_bptree_node(leaf->block,0);
    }
    else
    {
        free_data_block(data_id);
        free_key_block(key_id);
        return NULL;
    }

}

node *insert_into_leaf_after_splitting(node *root, node *leaf, int index, char *key, void *data)
{
    node *new_leaf;
    void **temp_ps;
    char **temp_ks, *new_key;
    unsigned long *temp_ps_block,*temp_ks_block;    


    int i, split;

    unsigned long new_key_id ;
    unsigned long key_id = get_bptree_keyid(data);
    
    unsigned long data_id = get_bptree_dataid(data); 
    
    if(key_id == -1 || data_id == -1)
    {
        free_data_block(data_id);
        free_key_block(key_id);
        return root;
    }


    temp_ps = malloc(size * sizeof(void *));
    temp_ks = malloc(size * sizeof(char *));
    temp_ps_block = malloc(size*sizeof(unsigned long));
    temp_ks_block = malloc(size*sizeof(unsigned long));


    for (i = 0; i < size; i++){
        if (i == index){
            temp_ps[i] = data;
            temp_ks[i] = malloc(MAX_KEY_LEN);
            strcpy(temp_ks[i], key);
            temp_ps_block[i] = data_id;
            temp_ks_block[i] = key_id;
        }
        else if (i < index){
            temp_ps[i] = leaf->pointers[i];
            temp_ks[i] = leaf->keys[i];
            temp_ps_block[i] = leaf->block->pointers_id[i];
            temp_ks_block[i] = leaf->block->keys_id[i];
        }
        else{
            temp_ps[i] = leaf->pointers[i-1];
            temp_ks[i] = leaf->keys[i-1];
            temp_ps_block[i] = leaf->block->pointers_id[i-1];
            temp_ks_block[i] = leaf->block->keys_id[i-1];
        }
    }

    split = size / 2;
    leaf->num_keys = split;
    leaf->block->num_keys = split;
    for (i = 0; i < split; i++){
        leaf->pointers[i] = temp_ps[i];
        leaf->keys[i] = temp_ks[i];
        leaf->block->pointers_id[i] = temp_ps_block[i];
        leaf->block->keys_id[i] = temp_ks_block[i];
    }

    new_leaf = make_new_leaf();
    new_leaf->num_keys = size - split;
    new_leaf->block->num_keys = size -split;
    for (; i < size; i++){
        new_leaf->pointers[i - split] = temp_ps[i];
        new_leaf->keys[i - split] = temp_ks[i];
        new_leaf->block->pointers_id[i - split] = temp_ps_block[i];
        new_leaf->block->keys_id[i - split] = temp_ks_block[i];
    }

    new_leaf->parent = leaf->parent;
    new_leaf->pointers[size - 1] = leaf->pointers[size - 1];
    new_leaf->block->parent_id = leaf->block->parent_id;
    leaf->pointers[size - 1] = new_leaf;
    leaf->block->pointers_id[size -1] = new_leaf->block->node_id;

    free(temp_ps);
    free(temp_ks);
    free(temp_ps_block);
    free(temp_ks_block);

    new_key = new_leaf->keys[0];
    new_key_id = new_leaf->block->keys_id[0]; 
    return insert_into_parent(root, leaf, new_leaf, new_key,new_key_id);
}

node *delete(node *root, char *key)
{
    node *leaf;
    int i;
    leaf = find_leaf(root, key);
    if (leaf == NULL)
        return root;
    for (i = 0; i < leaf->num_keys && strcmp(leaf->keys[i], key) != 0; i++)
        ;
    if (i == leaf->num_keys)  // no such key
        return root;
    root = delete_entry(root, leaf, i);
    return root;
}

node *delete_entry(node *root, node *nd, int index)
{
    int min_keys, cap, nd_index;
    node *neighbor;

    remove_entry(nd, index);
    if (nd == root)
        return adjust_root(nd);
    min_keys = nd->is_leaf ? size / 2 : (size - 1) / 2;
    if (nd->num_keys >= min_keys) {
        return root;
    }

    nd_index = get_node_index(nd);
    if (nd_index == 0)
        neighbor = nd->parent->pointers[1];  // right neighbor
    else
        neighbor = nd->parent->pointers[nd_index - 1]; // left neighbor

    cap = nd->is_leaf ? size - 1 : size - 2;
    if (neighbor->num_keys + nd->num_keys <= cap)
        return coalesce_nodes(root, nd, neighbor, nd_index);

    distribute_nodes(nd, neighbor, nd_index);
    return root;
}

void distribute_nodes(node *nd, node *neighbor, int nd_index)
{
    int i;
    node *tmp;
    if (nd_index != 0) {
        if (!nd->is_leaf)
            nd->pointers[nd->num_keys + 1] = nd->pointers[nd->num_keys];
        for (i = nd->num_keys; i > 0; i--){  // shift to right by 1
            nd->keys[i] = nd->keys[i - 1];
            nd->pointers[i] = nd->pointers[i - 1];
        }
        if (!nd->is_leaf){
            nd->keys[0] = nd->parent->keys[nd_index - 1];

            nd->pointers[0] = neighbor->pointers[neighbor->num_keys];
            tmp = (node *)nd->pointers[0];
            tmp->parent = nd;
            neighbor->pointers[neighbor->num_keys] = NULL;

            nd->parent->keys[nd_index - 1] = neighbor->keys[neighbor->num_keys - 1];
            neighbor->keys[neighbor->num_keys - 1] = NULL;
        }
        else {
            nd->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            neighbor->keys[neighbor->num_keys - 1] = NULL;

            nd->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
            neighbor->pointers[neighbor->num_keys - 1] = NULL;

            // nd->parent->keys[nd_index - 1] = nd->keys[0];  //  share the same key with child !!
            strcpy(nd->parent->keys[nd_index - 1], nd->keys[0]);
        }
    }
    else {
        if (!nd->is_leaf){
            nd->keys[nd->num_keys] = nd->parent->keys[0];  // link to father's key
            nd->pointers[nd->num_keys + 1] = neighbor->pointers[0];
            tmp = (node *)nd->pointers[nd->num_keys + 1];
            tmp->parent = nd;
            nd->parent->keys[0] = neighbor->keys[0];  //
        }
        else {
            nd->keys[nd->num_keys] = neighbor->keys[0];
            nd->pointers[nd->num_keys] = neighbor->pointers[0];
            // nd->parent->keys[0] = neighbor->keys[1];  // share the same key with chid !!
            strcpy(nd->parent->keys[0], neighbor->keys[1]);
        }
        for (i = 0; i < neighbor->num_keys - 1; i++){
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        neighbor->keys[i] = NULL;
        if (!nd->is_leaf)
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        else
            neighbor->pointers[i] = NULL;
    }

    neighbor->num_keys--;
    nd->num_keys++;

}

node *coalesce_nodes(node *root, node *nd, node *neighbor, int nd_index)
{
    int i, j, start, end;
    char *k_prime;
    node *tmp, *parent;

    if (nd_index == 0) {  // make sure neighbor is on the left
        tmp = nd;
        nd = neighbor;
        neighbor = tmp;
        nd_index = 1;
    }
    parent = nd->parent;

    start = neighbor->num_keys;
    if (nd->is_leaf){
        for (i = start, j = 0; j < nd->num_keys; i++, j++){
            neighbor->keys[i] = nd->keys[j];
            neighbor->pointers[i] = nd->pointers[j];
            nd->keys[j] = NULL;
            nd->pointers[j] = NULL;
        }
        neighbor->num_keys += nd->num_keys;
        neighbor->pointers[size - 1] = nd->pointers[size - 1];
    }
    else {
        neighbor->keys[start] = malloc(MAX_KEY_LEN);
        strcpy(neighbor->keys[start], parent->keys[nd_index - 1]);
        // neighbor->keys[start] = parent->keys[nd_index - 1];
        for (i = start + 1, j = 0; j < nd->num_keys; i++, j++){
            neighbor->keys[i] = nd->keys[j];
            neighbor->pointers[i] = nd->pointers[j];
        }
        neighbor->pointers[i] = nd->pointers[j];
        neighbor->num_keys += nd->num_keys + 1;

        for (i = 0; i <= neighbor->num_keys; i++){
            tmp = (node *)neighbor->pointers[i];
            tmp->parent = neighbor;
        }
    }
    destroy_node(nd);
    return delete_entry(root, parent, nd_index);
}

int get_node_index(node *nd)
{
    node *parent;
    int i;
    parent = nd->parent;
    for (i = 0; i < parent->num_keys && parent->pointers[i] != nd; i++)
        ;
    return i;
}

node *adjust_root(node *root)
{
    node *new_root;
    if (root->num_keys > 0)  // at least two childs
        return root;
    if (!root->is_leaf){  // root has only one child
        new_root = root->pointers[0];
        new_root->parent = NULL;
    }
    else
        new_root = NULL;
    destroy_node(root);
    return new_root;
}

void *remove_entry(node *nd, int index)
{
    int i, index_k;

    if (nd->is_leaf){
        free(nd->keys[index]);
        free(nd->pointers[index]);  // destroy the record
        for (i = index; i < nd->num_keys - 1; i++){
            nd->keys[i] = nd->keys[i + 1];
            nd->pointers[i] = nd->pointers[i + 1];
        }
        nd->keys[i] = NULL;
        nd->pointers[i] = NULL;
    }
    else{
        index_k = index - 1;  // index_p == index
        free(nd->keys[index_k]);
        for (i = index_k; i < nd->num_keys - 1; i++){
            nd->keys[i] = nd->keys[i + 1];
            nd->pointers[i + 1] = nd->pointers[i + 2];
        }
        nd->keys[i] = NULL;
        nd->pointers[i + 1] = NULL;
    }
    nd->num_keys--;
}

int cmp(const void *p, const void *q)
{
    //keys' type is (char **), each item is a (char *)
    return strcmp(*(char **)p, *(char **)q);
}

