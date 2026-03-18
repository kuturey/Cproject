#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Показать историю коммитов
void cmd_log(RepoState *repo) {
    if (!repo || !repo->head) {
        printf("No commits yet\n");
        return;
    }
    
    printf("\nCommit history:\n");
    printf("================\n");
    
    Commit *current = repo->head;
    int count = 0;
    
    while (current) {
        printf("\ncommit ");
        for(int i = 0; i < 8; i++) printf("%02x", current->hash[i]);
        printf("\n");
        
        // Проверяем наличие родителя (не все нули)
        int has_parent = 0;
        for (int i = 0; i < SHA1_HASH_SIZE; i++) {
            if (current->parent_hash[i] != 0) {
                has_parent = 1;
                break;
            }
        }
        
        if (has_parent) {
            printf("parent ");
            for(int i = 0; i < 8; i++) printf("%02x", current->parent_hash[i]);
            printf("\n");
        }
        
        printf("Date: %s", ctime(&current->timestamp));
        printf("    %s\n", current->message);
        
        // Переходим к родителю
        current = get_parent_commit(current, repo->store);
        count++;
        
        if (count > 100) break;
    }
}

// Показать конкретный коммит
void cmd_show(RepoState *repo, const char *commit_hash_str) {
    if (!repo || !commit_hash_str) return;
    
    // Конвертируем строку в хеш
    unsigned char hash[SHA1_HASH_SIZE];
    memset(hash, 0, SHA1_HASH_SIZE);
    
    // Проверяем длину строки
    if (strlen(commit_hash_str) < SHA1_HASH_SIZE * 2) {
        printf("Invalid hash length\n");
        return;
    }
    
    for (int i = 0; i < SHA1_HASH_SIZE; i++) {
        char byte_str[3] = {commit_hash_str[i*2], commit_hash_str[i*2+1], '\0'};
        hash[i] = (unsigned char)strtol(byte_str, NULL, 16);
    }
    
    // Ищем коммит
    Commit *commit = (Commit*)get_object(repo->store, hash);
    if (!commit) {
        printf("Commit not found\n");
        return;
    }
    
    // Печатаем коммит
    printf("\ncommit ");
    for(int i = 0; i < 8; i++) printf("%02x", commit->hash[i]);
    printf("\n");
    
    // Проверяем наличие родителя
    int has_parent = 0;
    for (int i = 0; i < SHA1_HASH_SIZE; i++) {
        if (commit->parent_hash[i] != 0) {
            has_parent = 1;
            break;
        }
    }
    
    if (has_parent) {
        printf("parent ");
        for(int i = 0; i < 8; i++) printf("%02x", commit->parent_hash[i]);
        printf("\n");
    }
    
    printf("Date: %s", ctime(&commit->timestamp));
    printf("    %s\n", commit->message);
    
    // Показываем дерево файлов
    Tree *tree = (Tree*)get_object(repo->store, commit->tree_hash);
    if (tree) {
        printf("\nFiles in this commit:\n");
        for (int i = 0; i < tree->entry_count; i++) {
            printf("  %s\n", tree->entries[i].name);
        }
    }
}