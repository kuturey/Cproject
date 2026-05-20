// катя
#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

void cmd_print_files(Commit *commit, ObjectStore *store) {
    if (!commit || !store) {
        printf("No files\n");
        return;
    }

    Tree *tree = get_commit_tree(commit, store);
    if (!tree || tree->entry_count == 0) {
        printf("No files\n");
        return;
    }

    for (int i = 0; i < tree->entry_count; i++) {
        TreeEntry *e = &tree->entries[i];

        printf("%s ", e->name);
        for (int j = 0; j < SHA1_HASH_SIZE; j++) {
            printf("%02x", e->hash[j]);
        }
        printf("\n");
    }
}