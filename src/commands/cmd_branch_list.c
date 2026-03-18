#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct BranchNode {
    char name[100];
    unsigned char commit_hash[SHA1_HASH_SIZE];
    struct BranchNode *next;
} BranchNode;

// ✅ УБРАЛИ static - теперь видна из других файлов
BranchNode *branches = NULL;

void init_branches(RepoState *repo);
int cmd_create_branch(RepoState *repo, const char *name);
int cmd_switch_branch(RepoState *repo, const char *name);
void cmd_branch_list(RepoState *repo);
int cmd_delete_branch(RepoState *repo, const char *name);
BranchNode* find_branch(const char *name);
void save_branches_to_disk(void);
void load_branches_from_disk(RepoState *repo);

void save_branches_to_disk(void) {
    FILE *f = fopen(".minigit/branches", "w");
    if (!f) return;
    
    BranchNode *current = branches;
    while (current) {
        fprintf(f, "%s ", current->name);
        for (int i = 0; i < SHA1_HASH_SIZE; i++) {
            fprintf(f, "%02x", current->commit_hash[i]);
        }
        fprintf(f, "\n");
        current = current->next;
    }
    fclose(f);
}

void load_branches_from_disk(RepoState *repo) {
    FILE *f = fopen(".minigit/branches", "r");
    if (!f) return;
    
    char name[100];
    char hash_str[41];
    
    while (fscanf(f, "%s %40s", name, hash_str) == 2) {
        BranchNode *node = (BranchNode*)malloc(sizeof(BranchNode));
        strcpy(node->name, name);
        
        for (int i = 0; i < SHA1_HASH_SIZE; i++) {
            sscanf(&hash_str[i*2], "%2hhx", &node->commit_hash[i]);
        }
        
        node->next = branches;
        branches = node;
    }
    fclose(f);
}

void init_branches(RepoState *repo) {
    load_branches_from_disk(repo);
    
    if (!branches) {
        branches = (BranchNode*)malloc(sizeof(BranchNode));
        strcpy(branches->name, "master");
        
        if (repo && repo->head) {
            memcpy(branches->commit_hash, repo->head->hash, SHA1_HASH_SIZE);
        } else {
            memset(branches->commit_hash, 0, SHA1_HASH_SIZE);
        }
        
        branches->next = NULL;
        
        if (repo) {
            repo->current_branch = strdup("master");
        }
    }
}

int cmd_create_branch(RepoState *repo, const char *name) {
    if (!repo || !name) {
        printf("Error: Invalid arguments\n");
        return -1;
    }
    
    BranchNode *current = branches;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            printf("Branch '%s' already exists\n", name);
            return -1;
        }
        current = current->next;
    }
    
    BranchNode *new_branch = (BranchNode*)malloc(sizeof(BranchNode));
    strcpy(new_branch->name, name);
    
    if (repo->head) {
        memcpy(new_branch->commit_hash, repo->head->hash, SHA1_HASH_SIZE);
    } else {
        memset(new_branch->commit_hash, 0, SHA1_HASH_SIZE);
    }
    
    new_branch->next = branches;
    branches = new_branch;
    
    printf("Branch '%s' created\n", name);
    
    save_branches_to_disk();
    save_repo_state(repo);
    
    return 0;
}

int cmd_switch_branch(RepoState *repo, const char *name) {
    if (!repo || !name) {
        printf("Error: Invalid arguments\n");
        return -1;
    }
    
    BranchNode *current = branches;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (repo->current_branch) {
                free(repo->current_branch);
            }
            repo->current_branch = strdup(name);
            
            Commit *target_commit = (Commit*)get_object(repo->store, current->commit_hash);
            if (target_commit) {
                repo->head = target_commit;
            }
            
            printf("Switched to branch '%s'\n", name);
            save_repo_state(repo);
            
            return 0;
        }
        current = current->next;
    }
    
    printf("Branch '%s' not found\n", name);
    return -1;
}

void cmd_branch_list(RepoState *repo) {
    if (!repo) return;
    
    if (!branches) {
        init_branches(repo);
    }
    
    printf("\nBranches:\n");
    BranchNode *current = branches;
    while (current) {
        if (repo->current_branch && strcmp(repo->current_branch, current->name) == 0) {
            printf("* %s", current->name);
        } else {
            printf("  %s", current->name);
        }
        
        printf(" [");
        for(int i = 0; i < 8; i++) {
            printf("%02x", current->commit_hash[i]);
        }
        printf("]\n");
        
        current = current->next;
    }
}

int cmd_delete_branch(RepoState *repo, const char *name) {
    if (!repo || !name) {
        printf("Error: Invalid arguments\n");
        return -1;
    }
    
    if (repo->current_branch && strcmp(repo->current_branch, name) == 0) {
        printf("Cannot delete current branch\n");
        return -1;
    }
    
    BranchNode *prev = NULL;
    BranchNode *current = branches;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                branches = current->next;
            }
            free(current);
            printf("Branch '%s' deleted\n", name);
            save_branches_to_disk();
            return 0;
        }
        prev = current;
        current = current->next;
    }
    
    printf("Branch '%s' not found\n", name);
    return -1;
}

BranchNode* find_branch(const char *name) {
    BranchNode *current = branches;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}