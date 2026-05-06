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
    Tree *base = clone_tree(get_commit_tree(repo->head, repo->store));
    Tree *nakl_branch = get_commit_tree(other, repo->store);
    TreeEntry *e;
    if (!base) base = create_tree();
    if (nakl_branch) {
        for (int i = 0; i < nakl_branch->entry_count; i++) {
            e = &nakl_branch->entries[i];
            add_tree_entry(base, e->name, e->type == BLOB_ENTRY, e->hash);
        }
    }
    if (repo->staging_area) free_tree(repo->staging_area);
    repo->staging_area = base;
    return cmd_commit(repo, message && strlen(message) ? message : "simple merge");
}