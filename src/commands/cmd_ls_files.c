#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

void cmd_ls_files(RepoState *repo) {
    if (!repo || !repo->staging_area) {
        printf("No files\n");
        return;
    }
    
    for (int i = 0; i < repo->staging_area->entry_count; i++) {
        TreeEntry *e = &repo->staging_area->entries[i];
        printf("%s\n", e->name);
    }
}