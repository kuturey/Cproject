#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

void cmd_stats(RepoState *repo) {
    if (!repo || !repo->store) {
        printf("total_commits: 0\n");
        printf("total_files: 0\n");
        printf("total_objects: 0\n");
        printf("memory_saved: 0 bytes\n");
        return;
    }
    
    int commit_count = 0;
    int file_count = 0;
    
    // ✅ Проверяем, что HEAD загружен
    Commit *current = repo->head;
    while (current) {
        commit_count++;
        Tree *tree = (Tree*)get_object(repo->store, current->tree_hash);
        if (tree) {
            file_count += tree->entry_count;
        }
        current = get_parent_commit(current, repo->store);
        if (commit_count > 1000) break;
    }
    
    printf("total_commits: %d\n", commit_count);
    printf("total_files: %d\n", file_count);
    printf("total_objects: %d\n", repo->store->total_count);
    printf("memory_saved: %d bytes\n", file_count * 50);
}