#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

Commit *get_branch_head(RepoState *repo, const char *name){
    BranchNode *branch = find_branch(name);
    return (Commit *)get_object(repo->store, branch->commit_hash);
}