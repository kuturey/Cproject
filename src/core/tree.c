#include "../include/minigit.h"
#include <stdlib.h>
#include <string.h>

void tree_update_hash(Tree *tree) {
    if (!tree) return;
    
    unsigned char data[sizeof(int) + tree->entry_count * sizeof(TreeEntry)];
    int offset = 0;
    
    memcpy(data + offset, &tree->entry_count, sizeof(int));
    offset += sizeof(int);
    
    for (int i = 0; i < tree->entry_count; i++) {
        memcpy(data + offset, &tree->entries[i], sizeof(TreeEntry));
        offset += sizeof(TreeEntry);
    }
    
    compute_hash(data, offset, tree->hash);
    
    ObjectStore *store = get_global_store();
    if (store) {
        add_object(store, tree, tree->hash);
    }
}

Tree* create_tree(void) {
    Tree *tree = (Tree*)calloc(1, sizeof(Tree));
    if (!tree) return NULL;
    
    tree->type = OBJ_TREE;  // ✅ ВАЖНО
    tree->entries = NULL;
    tree->entry_count = 0;
    memset(tree->hash, 0, SHA1_HASH_SIZE);
    
    return tree;
}

void add_tree_entry(Tree *tree, const char *name, int is_blob, const unsigned char *hash) {
    if (!tree) return;
    
    tree->entries = realloc(tree->entries,
        (tree->entry_count + 1) * sizeof(TreeEntry));
    
    TreeEntry *entry = &tree->entries[tree->entry_count];
    entry->name = strdup(name);
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

void print_tree(Tree *tree) {
    if (!tree) return;
    printf("\nTree contents:\n");
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