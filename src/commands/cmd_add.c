#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

int cmd_add(RepoState *repo, const char *path, const char *content) {
    if (!repo || !path || !content) {
        printf("Error: Invalid arguments\n");
        return -1;
    }
    
    printf("Adding file: %s\n", path);
    printf("Content: %s\n", content);
    
    // Создаем блоб
    Blob *blob = create_blob(content);
    if (!blob) {
        printf("Error: Failed to create blob\n");
        return -1;
    }
    
    // Если staging area нет, создаем
    if (!repo->staging_area) {
        printf("Creating new staging area\n");
        repo->staging_area = create_tree();
    }
    
    // Проверяем, есть ли уже такой файл в staging
    TreeEntry *existing = find_tree_entry(repo->staging_area, path);
    if (existing) {
        if (memcmp(existing->hash, blob->hash, SHA1_HASH_SIZE) == 0) {
            printf("File unchanged, nothing to stage\n");
            return 0;
        }
        printf("File updated in staging area\n");
    } else {
        printf("New file added to staging area\n");
    }
    
    // Добавляем в дерево staging
    add_tree_entry(repo->staging_area, path, 1, blob->hash);
    printf("Staging area now has %d files\n", repo->staging_area->entry_count);
    
    // СОХРАНЯЕМ СОСТОЯНИЕ СРАЗУ!
    save_repo_state(repo);
    return 0;
}

void print_staging_area(RepoState *repo) {
    if (!repo || !repo->staging_area || repo->staging_area->entry_count == 0) {
        printf("Staging area is empty\n");
        return;
    }
    
    printf("\nStaging area:\n");
    printf("  Files ready to commit:\n");
    
    for (int i = 0; i < repo->staging_area->entry_count; i++) {
        TreeEntry *e = &repo->staging_area->entries[i];
        printf("    %s (", e->name);
        for(int j = 0; j < 5; j++) printf("%02x", e->hash[j]);
        printf("...)\n");
    }
}