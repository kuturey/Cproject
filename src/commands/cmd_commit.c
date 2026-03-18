#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

extern BranchNode *branches;
void save_branches_to_disk(void);

Commit* cmd_commit(RepoState *repo, const char *message) {
    if (!repo) {
        printf("Error: No repository\n");
        return NULL;
    }
    
    printf("Staging area has %d files\n",
        repo->staging_area ? repo->staging_area->entry_count : 0);
    
    if (!repo->staging_area || repo->staging_area->entry_count == 0) {
        printf("Error: Nothing to commit (staging area is empty)\n");
        return NULL;
    }
    
    if (!message || strlen(message) == 0) {
        message = "No message";
    }
    
    printf("Committing changes...\n");
    
    Commit *new_commit = create_commit(repo->head, repo->staging_area, message);
    if (!new_commit) {
        printf("Error: Failed to create commit\n");
        return NULL;
    }
    
    // ✅ СОХРАНЯЕМ ВСЕ ОБЪЕКТЫ НА ДИСК
    save_object(repo->store, new_commit, OBJ_COMMIT, new_commit->hash);
    
    Tree *tree = (Tree*)get_object(repo->store, new_commit->tree_hash);
    if (tree) {
        save_object(repo->store, tree, OBJ_TREE, tree->hash);
        
        for (int i = 0; i < tree->entry_count; i++) {
            if (tree->entries[i].type == BLOB_ENTRY) {
                Blob *blob = (Blob*)get_object(repo->store, tree->entries[i].hash);
                if (blob) {
                    save_object(repo->store, blob, OBJ_BLOB, blob->hash);
                }
            }
        }
    }
    
    repo->head = new_commit;
    
    if (branches) {
        BranchNode *current = branches;
        while (current) {
            if (repo->current_branch && strcmp(current->name, repo->current_branch) == 0) {
                memcpy(current->commit_hash, new_commit->hash, SHA1_HASH_SIZE);
                break;
            }
            current = current->next;
        }
    }
    
    free_tree(repo->staging_area);
    repo->staging_area = create_tree();
    
    printf("Commit created successfully!\n");
    printf("   Hash: ");
    for(int i = 0; i < 8; i++) printf("%02x", new_commit->hash[i]);
    printf("\n");
    
    save_repo_state(repo);
    save_branches_to_disk();
    
    return new_commit;
}

void cmd_status(RepoState *repo) {
    if (!repo) return;
    
    printf("\nRepository status:\n");
    printf("==================\n");
    
    if (repo->current_branch) {
        printf("On branch: %s\n", repo->current_branch);
    } else {
        printf("Not on any branch\n");
    }
    
    if (repo->head) {
        printf("HEAD: ");
        for(int i = 0; i < 8; i++) printf("%02x", repo->head->hash[i]);
        printf("\n");
    } else {
        printf("HEAD: (no commits yet)\n");
    }
    
    print_staging_area(repo);
}