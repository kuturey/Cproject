#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

RepoState* init_repo(void) {
    RepoState *repo = (RepoState*)malloc(sizeof(RepoState));
    if (!repo) return NULL;
    
    repo->head = NULL;
    repo->current_branch = stringdup("master");
    repo->staging_area = create_tree();
    repo->store = init_object_store();
    
    load_all_objects_from_disk(repo->store, ".minigit/objects");
    
    load_repo_state(repo);
    
    init_branches(repo);
    
    return repo;
}

void free_repo(RepoState *repo) {
    if (!repo) return;
    
    save_repo_state(repo);
    
    if (repo->current_branch) free(repo->current_branch);
    if (repo->staging_area) free_tree(repo->staging_area);
    if (repo->store) free_object_store(repo->store);
    free(repo);
}

void set_head(RepoState *repo, Commit *commit) {
    repo->head = commit;
}

void reset_staging_to_head(RepoState *repo){
    if (repo->staging_area) free_tree(repo->staging_area);
    Tree *head_tree = get_commit_tree(repo->head, repo->store);
    repo->staging_area = clone_tree(head_tree);
    if (!repo->staging_area) repo->staging_area = create_tree();
}