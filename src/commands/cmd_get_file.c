// женя
#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

Commit *from_command_to_commit(RepoState *repo, const char *hash_str) {
    if (!repo || !repo->store) return NULL;

    if (!hash_str || strcmp(hash_str, "HEAD") == 0) {
        return repo->head;
    }

    unsigned char hash[SHA1_HASH_SIZE];

    if (!parse_hash(hash_str, hash)) {
        return NULL;
    }

    return (Commit*)get_object(repo->store, hash);
}

void cmd_content(RepoState *repo, const char *commit_hash, const char *path) {
    Commit *commit = from_command_to_commit(repo, commit_hash);
    if (!commit) {
        printf("Commit not found\n");
        return;
    }
    char *content = get_file_content(commit, path, repo->store);
    if (!content) {
        printf("File not found\n");
        return;
    }
    printf("%s\n", content);
    free(content);
}

void cmd_exists(RepoState *repo, const char *commit_hash, const char *path) {
    Commit *commit = from_command_to_commit(repo, commit_hash);
    if (!commit) {
        printf("false\n");
        return;
    }
    printf(get_file_exists(commit, path, repo->store) ? "true\n" : "false\n");
}