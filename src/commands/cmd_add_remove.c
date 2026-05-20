#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

void ensure_has_staging(RepoState *repo) {
    if (!repo->staging_area) reset_staging_to_head(repo);
}

int cmd_add(RepoState *repo, const char *path, const char *content) {
    
    printf("Adding file: %s\n", path);
    printf("Content: %s\n", content);
    
    ensure_has_staging(repo);
    Blob *blob = create_blob(content);
    
    if (!repo->staging_area) {
        printf("Creating new staging area\n");
        repo->staging_area = create_tree();
    }
    
    TreeEntry *existing = find_tree_entry(repo->staging_area, path);
    if (existing && memcmp(existing->hash, blob->hash, SHA1_HASH_SIZE) == 0) {
        printf("File unchanged: %s\n", path);
        return 0;
    }

    add_tree_entry(repo->staging_area, path, 1, blob->hash);
    save_object(repo->store, blob, OBJ_BLOB, blob->hash);
    printf(existing ? "Updated: %s\n" : "Added: %s\n", path);
    save_repo_state(repo);
    return 0;
}

int cmd_remove(RepoState *repo, const char *path){
    
    ensure_has_staging(repo);
    if (remove_tree_entry(repo->staging_area, path)) {
        printf("Removed: %s\n", path);
        save_repo_state(repo);
        return 0;
    }
    printf("File not found: %s\n", path);
    return 0;
}
