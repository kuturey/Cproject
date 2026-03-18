#include "../include/minigit.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

Commit* create_commit(Commit *parent, Tree *root, const char *message) {
    // ✅ Используем calloc и устанавливаем type
    Commit *commit = (Commit*)calloc(1, sizeof(Commit));
    if (!commit) return NULL;
    
    commit->type = OBJ_COMMIT;  // ✅ ВАЖНО
    
    if (root) {
        memcpy(commit->tree_hash, root->hash, SHA1_HASH_SIZE);
    } else {
        memset(commit->tree_hash, 0, SHA1_HASH_SIZE);
    }
    
    if (parent) {
        memcpy(commit->parent_hash, parent->hash, SHA1_HASH_SIZE);
    } else {
        memset(commit->parent_hash, 0, SHA1_HASH_SIZE);
    }
    
    commit->message = strdup(message);
    commit->timestamp = time(NULL);
    
    unsigned char data[SHA1_HASH_SIZE * 2 + 256 + sizeof(time_t)];
    int offset = 0;
    memcpy(data + offset, commit->tree_hash, SHA1_HASH_SIZE);
    offset += SHA1_HASH_SIZE;
    memcpy(data + offset, commit->parent_hash, SHA1_HASH_SIZE);
    offset += SHA1_HASH_SIZE;
    size_t msg_len = strlen(message);
    memcpy(data + offset, message, msg_len);
    offset += msg_len;
    memcpy(data + offset, &commit->timestamp, sizeof(time_t));
    offset += sizeof(time_t);
    
    compute_hash(data, offset, commit->hash);
    
    ObjectStore *store = get_global_store();
    if (store) {
        add_object(store, commit, commit->hash);
    }
    
    return commit;
}

Commit* get_parent_commit(Commit *commit, ObjectStore *store) {
    if (!commit || !store) return NULL;
    
    int has_parent = 0;
    for (int i = 0; i < SHA1_HASH_SIZE; i++) {
        if (commit->parent_hash[i] != 0) {
            has_parent = 1;
            break;
        }
    }
    if (!has_parent) return NULL;
    
    return (Commit*)get_object(store, commit->parent_hash);
}

void print_commit(Commit *commit) {
    if (!commit) return;
    
    printf("\ncommit ");
    for(int i = 0; i < 8; i++) printf("%02x", commit->hash[i]);
    printf("\n");
    
    int has_parent = 0;
    for (int i = 0; i < SHA1_HASH_SIZE; i++) {
        if (commit->parent_hash[i] != 0) {
            has_parent = 1;
            break;
        }
    }
    if (has_parent) {
        printf("parent ");
        for(int i = 0; i < 8; i++) printf("%02x", commit->parent_hash[i]);
        printf("\n");
    }
    
    printf("Date: %s", ctime(&commit->timestamp));
    printf("    %s\n", commit->message);
}

void print_history(Commit *start, ObjectStore *store) {
    if (!start) return;
    
    printf("\nCommit History:\n");
    printf("================\n");
    
    Commit *current = start;
    int count = 0;
    while (current) {
        printf("\n%d. ", ++count);
        print_commit(current);
        current = get_parent_commit(current, store);
        if (count > 100) break;
    }
}

void free_commit(Commit *commit) {
    if (commit) {
        if (commit->message) {
            free(commit->message);
        }
        free(commit);
    }
}