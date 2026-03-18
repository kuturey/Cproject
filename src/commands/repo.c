#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

RepoState* init_repo(void) {
    RepoState *repo = (RepoState*)malloc(sizeof(RepoState));
    if (!repo) return NULL;
    
    repo->head = NULL;
    repo->current_branch = strdup("master");
    repo->staging_area = create_tree();
    repo->store = init_object_store();
    
    // ✅ 1. СНАЧАЛА загружаем все объекты с диска
    load_all_objects_from_disk(repo->store, ".minigit/objects");
    
    // ✅ 2. ПОТОМ загружаем состояние (HEAD, staging, ветки)
    load_repo_state(repo);
    
    // ✅ 3. Инициализируем ветки
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