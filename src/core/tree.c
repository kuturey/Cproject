#include "../include/minigit.h"
#include <stdlib.h>
#include <string.h>

int compare_tree_entries(const void *a, const void *b) {
    const TreeEntry *ea = (const TreeEntry*)a;
    const TreeEntry *eb = (const TreeEntry*)b;
    return strcmp(ea->name, eb->name);
}

void tree_update_hash(Tree *tree) {
    if (!tree) return;

    if (tree->entry_count > 1) {
        qsort(tree->entries, tree->entry_count, sizeof(TreeEntry), compare_tree_entries);
    }

    int total_size = sizeof(int); 

    for (int i = 0; i < tree->entry_count; i++) {
        int name_len = (int)strlen(tree->entries[i].name);
        total_size += sizeof(int) + name_len + sizeof(int) + SHA1_HASH_SIZE;
    }

    unsigned char *data = (unsigned char*)malloc(total_size);

    int offset = 0;

    memcpy(data + offset, &tree->entry_count, sizeof(int));
    offset += sizeof(int);

    for (int i = 0; i < tree->entry_count; i++) {
        int name_len = (int)strlen(tree->entries[i].name);
        int type = (int)tree->entries[i].type;

        memcpy(data + offset, &name_len, sizeof(int));
        offset += sizeof(int);

        memcpy(data + offset, tree->entries[i].name, name_len);
        offset += name_len;

        memcpy(data + offset, &type, sizeof(int));
        offset += sizeof(int);

        memcpy(data + offset, tree->entries[i].hash, SHA1_HASH_SIZE);
        offset += SHA1_HASH_SIZE;
    }

    compute_hash(data, offset, tree->hash);
    free(data);
}

Tree* create_tree(void) {
    Tree *tree = (Tree*)calloc(1, sizeof(Tree));
    if (!tree) return NULL;
    
    tree->type = OBJ_TREE;  
    tree->entries = NULL;
    tree->entry_count = 0;
    memset(tree->hash, 0, SHA1_HASH_SIZE);
    
    return tree;
}

Tree *clone_tree(Tree *tree){
    if (!tree){
        return create_tree();
    }

    Tree *copy = create_tree();

    if (tree->entry_count > 0){
        copy->entries = (TreeEntry *)calloc(tree->entry_count, sizeof(TreeEntry));
    }

    copy->entry_count = tree->entry_count;

    for (int i = 0; i < tree->entry_count; i++){
        copy->entries[i].name = stringdup(tree->entries[i].name);

        copy->entries[i].type = tree->entries[i].type;
        memcpy(copy->entries[i].hash, tree->entries[i].hash, SHA1_HASH_SIZE);
    }


    tree_update_hash(copy);
    return copy;
}

void add_tree_entry(Tree *tree, const char *name, int is_blob, const unsigned char *hash) {
    // обновление файла если уже есть с таким именем
    for (int i = 0; i < tree->entry_count; i++){
        if (strcmp(tree->entries[i].name, name) == 0){
            tree->entries[i].type = is_blob ? BLOB_ENTRY : TREE_ENTRY;
            memcpy(tree->entries[i].hash, hash, SHA1_HASH_SIZE);
            tree_update_hash(tree);
            return;
        }
    }


    tree->entries = realloc(tree->entries,
        (tree->entry_count + 1) * sizeof(TreeEntry));
    if (!tree->entries) return;    
    
    TreeEntry *entry = &tree->entries[tree->entry_count];
    entry->name = stringdup(name);
    if (!entry->name) return;

    entry->type = is_blob ? BLOB_ENTRY : TREE_ENTRY;
    memcpy(entry->hash, hash, SHA1_HASH_SIZE);
    
    tree->entry_count++;
    tree_update_hash(tree);
}

TreeEntry* find_tree_entry(Tree *tree, const char *name) {
    for (int i = 0; i < tree->entry_count; i++) {
        if (strcmp(tree->entries[i].name, name) == 0) {
            return &tree->entries[i];
        }
    }
    return NULL;
}

int remove_tree_entry(Tree *tree, const char *name){
    int idx = -1;
    for (int i = 0; i < tree->entry_count; i++){
        if (strcmp(tree->entries[i].name, name) == 0){
            idx = i;
            break;
        }
    }

    if (idx == -1) return 0;

    free(tree->entries[idx].name);
    
    for (int i = idx; i < tree->entry_count - 1; i++){
        tree->entries[i] = tree->entries[i + 1];
    }

    tree->entry_count--;

    if (tree->entry_count == 0){
        free(tree->entries);
        tree->entries = NULL;
    }
    else{
        tree->entries = (TreeEntry *)realloc(tree->entries, tree->entry_count * sizeof(TreeEntry));
    }

    tree_update_hash(tree);
    return 1;
}

Tree* get_commit_tree(Commit *commit, ObjectStore *store) {
    return (Tree*)get_object(store, commit->tree_hash);
}

int get_file_exists(Commit *commit, const char *path, ObjectStore *store) {
    Tree *tree = get_commit_tree(commit, store);

    TreeEntry *entry = find_tree_entry(tree, path);

    return entry->type == BLOB_ENTRY;
}

char* get_file_content(Commit *commit, const char *path, ObjectStore *store) {

    Tree *tree = get_commit_tree(commit, store);

    TreeEntry *entry = find_tree_entry(tree, path);

    Blob *blob = (Blob*)get_object(store, entry->hash);

    return stringdup(blob->content);
}

void print_files(Commit *commit, ObjectStore *store) {

    Tree *tree = get_commit_tree(commit, store);

    printf("Files in commit:\n");
    for (int i = 0; i < tree->entry_count; i++) {
        TreeEntry *e = &tree->entries[i];
        printf("  %s  ", e->name);

        for (int j = 0; j < 8; j++) {
            printf("%02x", e->hash[j]);
        }

        printf("  (%s)\n", e->type == BLOB_ENTRY ? "blob" : "tree");
    }
}

void print_tree(Tree *tree) {
    if (!tree) return;
    printf("\nTree:\n");
    printf("  Hash: ");
    for(int i = 0; i < 8; i++) printf("%02x", tree->hash[i]);
    printf("\n");
    printf("  Entries: %d\n", tree->entry_count);
    for (int i = 0; i < tree->entry_count; i++) {
        TreeEntry *e = &tree->entries[i];
        printf("  %s: ", e->name);
        for(int j = 0; j < 5; j++) printf("%02x", e->hash[j]);
        printf(" (%s)\n", e->type == BLOB_ENTRY ? "blob" : "tree");
    }
}

void free_tree(Tree *tree) {
    if (tree) {
        for (int i = 0; i < tree->entry_count; i++) {
            free(tree->entries[i].name);
        }
        free(tree->entries);
        free(tree);
    }
}

int tree_equals(Tree *a, Tree *b) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    tree_update_hash(a);
    tree_update_hash(b);
    return memcmp(a->hash, b->hash, SHA1_HASH_SIZE) == 0;
}