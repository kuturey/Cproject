#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

void cmd_ls_tree(RepoState *repo, const char *hash_str) {
    if (!repo || !repo->store || !hash_str) {
        printf("Error: Invalid arguments\n");
        return;
    }
    
    // Конвертируем строку в хеш
    unsigned char hash[SHA1_HASH_SIZE];
    memset(hash, 0, SHA1_HASH_SIZE);
    
    for (int i = 0; i < SHA1_HASH_SIZE && i*2 < (int)strlen(hash_str); i++) {
        char byte_str[3] = {hash_str[i*2], hash_str[i*2+1], '\0'};
        hash[i] = (unsigned char)strtol(byte_str, NULL, 16);
    }
    
    // Ищем дерево
    Tree *tree = (Tree*)get_object(repo->store, hash);
    if (!tree) {
        printf("Tree not found\n");
        return;
    }
    
    // Выводим файлы
    for (int i = 0; i < tree->entry_count; i++) {
        TreeEntry *e = &tree->entries[i];
        printf("%s ", e->name);
        for(int j = 0; j < 8; j++) printf("%02x", e->hash[j]); // только первые 8 байт
        printf("\n");
    }
}