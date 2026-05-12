#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

Commit* cmd_simple_merge(RepoState *repo, const char *other_branch, const char *message) {
    if (!repo || !other_branch) return NULL;

    Commit *other = get_branch_head(repo, other_branch);
    if (!other) {
        printf("Branch or branch head not found: %s\n", other_branch);
        return NULL;
    }

    Tree *exit = create_tree();
    if (!exit) return NULL;

    Tree *current_tree = get_commit_tree(repo->head, repo->store);
    TreeEntry *e;
    if (current_tree) {
        for (int i = 0; i < current_tree->entry_count; i++) {
            e = &current_tree->entries[i];
            add_tree_entry(exit, e->name, e->type == BLOB_ENTRY, e->hash);
        }
    }

    Tree *other_tree = get_commit_tree(other, repo->store);
    if (other_tree) {
        for (int i = 0; i < other_tree->entry_count; i++) {
            e = &other_tree->entries[i];
            add_tree_entry(exit, e->name, e->type == BLOB_ENTRY, e->hash);
        }
    }

    if (repo->staging_area) free_tree(repo->staging_area);
    repo->staging_area = exit;

    return cmd_commit(repo, message && strlen(message) ? message : "simple merge");
}