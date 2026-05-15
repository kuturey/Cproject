#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>

// Удалить файл из staging area
int cmd_remove(RepoState *repo, const char *path) {
    if (!repo || !path) {
        printf("Error: Invalid arguments\n");
        return -1;
    }
    
    if (!repo->staging_area || repo->staging_area->entry_count == 0) {
        printf("Error: Staging area is empty\n");
        return -1;
    }
    
    // Проверяем, есть ли файл в staging
    TreeEntry *existing = find_tree_entry(repo->staging_area, path);
    if (!existing) {
        printf("Error: File '%s' not in staging area\n", path);
        return -1;
    }
    
    // Удаляем из staging area
    remove_tree_entry(repo->staging_area, path);
    
    printf("File '%s' removed from staging area\n", path);
    printf("Staging area now has %d files\n", repo->staging_area->entry_count);
    
    // Сохраняем состояние
    save_repo_state(repo);
    
    return 0;
}

// Удалить файл из коммита (создать новый коммит без файла)
int cmd_remove_from_commit(RepoState *repo, const char *path, const char *message) {
    if (!repo || !path) {
        printf("Error: Invalid arguments\n");
        return -1;
    }
    
    if (!repo->head) {
        printf("Error: No commits yet\n");
        return -1;
    }
    
    // Получаем текущее дерево
    Tree *current_tree = (Tree*)get_object(repo->store, repo->head->tree_hash);
    if (!current_tree) {
        printf("Error: Failed to get current tree\n");
        return -1;
    }
    
    // Проверяем, есть ли файл в дереве
    if (!tree_entry_exists(current_tree, path)) {
        printf("Error: File '%s' not found in current commit\n", path);
        return -1;
    }
    
    // Создаем новое дерево (копируем текущее)
    Tree *new_tree = create_tree();
    
    // Копируем все записи кроме удаляемой
    for (int i = 0; i < current_tree->entry_count; i++) {
        TreeEntry *e = &current_tree->entries[i];
        if (strcmp(e->name, path) != 0) {
            add_tree_entry(new_tree, e->name, e->type == BLOB_ENTRY, e->hash);
        }
    }
    
    printf("File '%s' removed from tree\n", path);
    
    // Создаем новый коммит с обновленным деревом
    Commit *new_commit = create_commit(repo->head, new_tree, message);
    if (!new_commit) {
        printf("Error: Failed to create commit\n");
        free_tree(new_tree);
        return -1;
    }
    
    // Сохраняем объекты на диск
    save_object(repo->store, new_commit, OBJ_COMMIT, new_commit->hash);
    save_object(repo->store, new_tree, OBJ_TREE, new_tree->hash);
    
    // Обновляем HEAD
    repo->head = new_commit;
    
    // Обновляем хеш ветки
    extern BranchNode *branches;
    if (branches) {
        BranchNode *current = branches;
        while (current) {
            if (repo->current_branch && strcmp(current->name, repo->current_branch) == 0) {
                memcpy(current->commit_hash, new_commit->hash, SHA1_HASH_SIZE);
                break;
            }
            current = current->next;
        }
    }
    
    printf("Commit created successfully!\n");
    printf("   Hash: ");
    for(int i = 0; i < 8; i++) printf("%02x", new_commit->hash[i]);
    printf("\n");
    
    save_repo_state(repo);
    save_branches_to_disk();
    
    return 0;
}