#include "../include/minigit.h"
#include <stdlib.h>
#include <string.h>

Blob* create_blob(const char *content) {
    if (!content) return NULL;
    
    unsigned char temp_hash[SHA1_HASH_SIZE];
    compute_hash(content, strlen(content), temp_hash);
    
    ObjectStore *store = get_global_store();
    if (store) {
        Blob *existing = (Blob*)get_object(store, temp_hash);
        if (existing) {
            return existing;
        }
    }
    
    Blob *blob = (Blob*)calloc(1, sizeof(Blob));
    if (!blob) return NULL;
    
    blob->type = OBJ_BLOB;  // ✅ ВАЖНО
    blob->size = strlen(content);
    blob->content = (char*)malloc(blob->size + 1);
    if (!blob->content) {
        free(blob);
        return NULL;
    }
    strcpy(blob->content, content);
    memcpy(blob->hash, temp_hash, SHA1_HASH_SIZE);
    
    if (store) {
        add_object(store, blob, blob->hash);
    }
    
    return blob;
}

void free_blob(Blob *blob) {
    if (!blob) return;
    if (blob->content) {
        free(blob->content);
    }
    free(blob);
}