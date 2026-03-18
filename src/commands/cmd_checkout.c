#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Декларация функции поиска ветки из cmd_branch_list.c
struct BranchNode* find_branch(const char *name);
int cmd_switch_branch(RepoState *repo, const char *branch_name);

// Переключиться на коммит или ветку
int cmd_checkout(RepoState *repo, const char *target) {
    if (!repo || !target) return -1;
    
    printf("\nChecking out: %s\n", target);
    
    // Сначала пробуем как ветку
    struct BranchNode *branch = find_branch(target);
    if (branch) {
        return cmd_switch_branch(repo, target);
    }
    
    // Если не ветка, пробуем как хеш коммита
    unsigned char hash[SHA1_HASH_SIZE];
    memset(hash, 0, SHA1_HASH_SIZE);
    
    if (strlen(target) >= SHA1_HASH_SIZE * 2) {
        for (int i = 0; i < SHA1_HASH_SIZE; i++) {
            char byte_str[3] = {target[i*2], target[i*2+1], '\0'};
            hash[i] = (unsigned char)strtol(byte_str, NULL, 16);
        }
        
        Commit *target_commit = (Commit*)get_object(repo->store, hash);
        if (target_commit) {
            repo->head = target_commit;
            repo->current_branch = NULL;  // detached HEAD
            printf("Switched to commit\n");
            return 0;
        }
    }
    
    printf("Error: '%s' not found\n", target);
    return -1;
}