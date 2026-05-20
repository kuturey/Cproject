// женя
#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

Commit *get_branch_head(RepoState *repo, const char *name){
    if (!name || !repo){
        return NULL;
    }
    BranchNode *branch = find_branch(name);
    if (!branch){
        return NULL;
    }
    return (Commit *)get_object(repo->store, branch->commit_hash);
}