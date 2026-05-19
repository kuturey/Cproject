#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

            if (repo->current_branch) {
                free(repo->current_branch);
                repo->current_branch = NULL;
            }

            /* Синхронизируем staging_area с деревом коммита,
             * иначе add/commit продолжат работать со старыми файлами */
            if (repo->staging_area) free_tree(repo->staging_area);
            Tree *commit_tree = get_commit_tree(target_commit, repo->store);
            repo->staging_area = commit_tree ? clone_tree(commit_tree) : create_tree();

            save_repo_state(repo);

            printf("Switched to commit\n");
            return 0;
        }
    }
    
    printf("Error: '%s' not found\n", target);
    return -1;
}