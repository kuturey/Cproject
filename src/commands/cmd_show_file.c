#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

int cmd_show_file(RepoState *repo, const char *path) {
    if (!repo || !path) {
        printf("Error: Invalid arguments\n");
        return -1;
    }
    if (!repo->head) {
        printf("Error: No commits yet\n");
        return -1;
    }
    if (!get_file_exists(repo->head, repo->store, path)) {
        printf("Error: File '%s' not found in current commit\n", path);
        return -1;
    }
    char *content = get_file_content(repo->head, repo->store, path);
    if (!content) {
        printf("Error: Failed to get file content\n");
        return -1;
    }
    printf("=== %s ===\n", path);
    printf("%s\n", content);
    return 0;
}

int cmd_show_file_at_commit(RepoState *repo, const char *commit_hash, const char *path) {
    if (!repo || !commit_hash || !path) {
        printf("Error: Invalid arguments\n");
        return -1;
    }
    unsigned char hash[SHA1_HASH_SIZE];
    memset(hash, 0, SHA1_HASH_SIZE);
    for (int i = 0; i < SHA1_HASH_SIZE && i*2 < (int)strlen(commit_hash); i++) {
        char byte_str[3] = {commit_hash[i*2], commit_hash[i*2+1], '\0'};
        hash[i] = (unsigned char)strtol(byte_str, NULL, 16);
    }
    Commit *commit = (Commit*)get_object(repo->store, hash);
    if (!commit) {
        printf("Error: Commit not found\n");
        return -1;
    }
    if (!get_file_exists(commit, repo->store, path)) {
        printf("Error: File '%s' not found in commit %s\n", path, commit_hash);
        return -1;
    }
    char *content = get_file_content(commit, repo->store, path);
    if (!content) {
        printf("Error: Failed to get file content\n");
        return -1;
    }
    printf("=== %s @ %s ===\n", path, commit_hash);
    printf("%s\n", content);
    return 0;
}