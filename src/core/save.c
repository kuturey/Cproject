#include "../include/minigit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#define MINIGIT_DIR ".minigit"
#define OBJECTS_DIR ".minigit/objects"
#define HEAD_FILE ".minigit/HEAD"
#define BRANCH_FILE ".minigit/current_branch"
#define STAGING_FILE ".minigit/staging"

void init_dirs(void) {
    mkdir(MINIGIT_DIR);
    mkdir(OBJECTS_DIR);
}

// ✅ ИСПРАВЛЕНО: Принимаем тип объекта явно
void save_object(ObjectStore *store, void *obj, ObjectType type, const unsigned char *hash) {
    char path[512];
    char hash_str[41];
    
    for (int i = 0; i < SHA1_HASH_SIZE; i++) {
        sprintf(&hash_str[i*2], "%02x", hash[i]);
    }
    hash_str[40] = '\0';
    
    char dir_path[512];
    sprintf(dir_path, "%s/%.2s", OBJECTS_DIR, hash_str);
    mkdir(dir_path);
    
    sprintf(path, "%s/%s", dir_path, hash_str + 2);
    
    // Не перезаписываем существующие
    if (access(path, F_OK) == 0) return;
    
    FILE *f = fopen(path, "wb");
    if (!f) return;
    
    // ✅ 1 байт типа объекта
    unsigned char type_byte = (unsigned char)type;
    fwrite(&type_byte, 1, 1, f);
    
    // ✅ Сохраняем в зависимости от типа
    if (type == OBJ_BLOB) {
        Blob *blob = (Blob*)obj;
        fwrite(&blob->size, sizeof(size_t), 1, f);
        if (blob->size > 0 && blob->content) {
            fwrite(blob->content, blob->size, 1, f);
        }
    }
    else if (type == OBJ_TREE) {
        Tree *tree = (Tree*)obj;
        fwrite(&tree->entry_count, sizeof(int), 1, f);
        for (int i = 0; i < tree->entry_count; i++) {
            TreeEntry *e = &tree->entries[i];
            int name_len = strlen(e->name) + 1;
            fwrite(&name_len, sizeof(int), 1, f);
            fwrite(e->name, name_len, 1, f);
            fwrite(&e->type, sizeof(int), 1, f);
            fwrite(e->hash, SHA1_HASH_SIZE, 1, f);
        }
    }
    else if (type == OBJ_COMMIT) {
        Commit *commit = (Commit*)obj;
        fwrite(commit->tree_hash, SHA1_HASH_SIZE, 1, f);
        fwrite(commit->parent_hash, SHA1_HASH_SIZE, 1, f);
        int msg_len = strlen(commit->message) + 1;
        fwrite(&msg_len, sizeof(int), 1, f);
        fwrite(commit->message, msg_len, 1, f);
        fwrite(&commit->timestamp, sizeof(time_t), 1, f);
    }
    
    fclose(f);
}

void* load_object(ObjectStore *store, const unsigned char *hash) {
    char path[512];
    char hash_str[41];
    
    for (int i = 0; i < SHA1_HASH_SIZE; i++) {
        sprintf(&hash_str[i*2], "%02x", hash[i]);
    }
    hash_str[40] = '\0';
    
    sprintf(path, "%s/%.2s/%s", OBJECTS_DIR, hash_str, hash_str + 2);
    
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    
    unsigned char type_byte;
    if (fread(&type_byte, 1, 1, f) != 1) {
        fclose(f);
        return NULL;
    }
    ObjectType type = (ObjectType)type_byte;
    
    void *obj = NULL;
    
    if (type == OBJ_BLOB) {
        Blob *blob = (Blob*)calloc(1, sizeof(Blob));
        blob->type = OBJ_BLOB;
        fread(&blob->size, sizeof(size_t), 1, f);
        if (blob->size > 0) {
            blob->content = (char*)malloc(blob->size + 1);
            fread(blob->content, blob->size, 1, f);
            blob->content[blob->size] = '\0';
        }
        memcpy(blob->hash, hash, SHA1_HASH_SIZE);
        obj = blob;
    }
    else if (type == OBJ_TREE) {
        Tree *tree = (Tree*)calloc(1, sizeof(Tree));
        tree->type = OBJ_TREE;
        fread(&tree->entry_count, sizeof(int), 1, f);
        if (tree->entry_count > 0) {
            tree->entries = (TreeEntry*)calloc(tree->entry_count, sizeof(TreeEntry));
            for (int i = 0; i < tree->entry_count; i++) {
                TreeEntry *e = &tree->entries[i];
                int name_len;
                fread(&name_len, sizeof(int), 1, f);
                e->name = (char*)malloc(name_len);
                fread(e->name, name_len, 1, f);
                fread(&e->type, sizeof(int), 1, f);
                fread(e->hash, SHA1_HASH_SIZE, 1, f);
            }
        }
        memcpy(tree->hash, hash, SHA1_HASH_SIZE);
        obj = tree;
    }
    else if (type == OBJ_COMMIT) {
        Commit *commit = (Commit*)calloc(1, sizeof(Commit));
        commit->type = OBJ_COMMIT;
        fread(commit->tree_hash, SHA1_HASH_SIZE, 1, f);
        fread(commit->parent_hash, SHA1_HASH_SIZE, 1, f);
        int msg_len;
        fread(&msg_len, sizeof(int), 1, f);
        commit->message = (char*)malloc(msg_len);
        fread(commit->message, msg_len, 1, f);
        fread(&commit->timestamp, sizeof(time_t), 1, f);
        memcpy(commit->hash, hash, SHA1_HASH_SIZE);
        obj = commit;
    }
    
    fclose(f);
    
    if (store && obj) {
        add_object(store, obj, hash);
    }
    
    return obj;
}

// ✅ Загрузить ВСЕ объекты с диска
void load_all_objects_from_disk(ObjectStore *store, const char *objects_dir) {
    DIR *dir = opendir(objects_dir);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char subdir_path[512];
        snprintf(subdir_path, sizeof(subdir_path), "%s/%s", objects_dir, entry->d_name);
        
        DIR *subdir = opendir(subdir_path);
        if (!subdir) continue;
        
        struct dirent *subentry;
        while ((subentry = readdir(subdir)) != NULL) {
            if (subentry->d_name[0] == '.') continue;
            
            char full_hash[41];
            snprintf(full_hash, sizeof(full_hash), "%s%s", entry->d_name, subentry->d_name);
            
            unsigned char hash[SHA1_HASH_SIZE];
            for (int i = 0; i < SHA1_HASH_SIZE; i++) {
                sscanf(&full_hash[i*2], "%2hhx", &hash[i]);
            }
            
            load_object(store, hash);
        }
        
        closedir(subdir);
    }
    
    closedir(dir);
}

// ✅ Загрузить staging area с диска
void load_staging_area(RepoState *repo) {
    FILE *f = fopen(STAGING_FILE, "r");
    if (!f) return;
    
    char name[260];
    char hash_str[41];
    
    while (fscanf(f, "%s %40s", name, hash_str) == 2) {
        unsigned char hash[SHA1_HASH_SIZE];
        for (int i = 0; i < SHA1_HASH_SIZE; i++) {
            sscanf(&hash_str[i*2], "%2hhx", &hash[i]);
        }
        add_tree_entry(repo->staging_area, name, 1, hash);
    }
    
    fclose(f);
}

int save_repo_state(RepoState *repo) {
    init_dirs();
    
    if (repo->head) {
        FILE *f = fopen(HEAD_FILE, "w");
        if (f) {
            for (int i = 0; i < SHA1_HASH_SIZE; i++) {
                fprintf(f, "%02x", repo->head->hash[i]);
            }
            fprintf(f, "\n");
            fclose(f);
        }
    }
    
    if (repo->current_branch) {
        FILE *f = fopen(BRANCH_FILE, "w");
        if (f) {
            fprintf(f, "%s\n", repo->current_branch);
            fclose(f);
        }
    }
    
    if (repo->staging_area && repo->staging_area->entry_count > 0) {
        FILE *f = fopen(STAGING_FILE, "w");
        if (f) {
            for (int i = 0; i < repo->staging_area->entry_count; i++) {
                TreeEntry *e = &repo->staging_area->entries[i];
                fprintf(f, "%s ", e->name);
                for (int j = 0; j < SHA1_HASH_SIZE; j++) {
                    fprintf(f, "%02x", e->hash[j]);
                }
                fprintf(f, "\n");
            }
            fclose(f);
        }
    }
    
    return 0;
}

int load_repo_state(RepoState *repo) {
    init_dirs();
    
    FILE *f = fopen(HEAD_FILE, "r");
    if (f) {
        char hash_str[41] = {0};
        if (fgets(hash_str, 41, f)) {
            hash_str[40] = '\0';
            unsigned char hash[SHA1_HASH_SIZE];
            for (int i = 0; i < SHA1_HASH_SIZE; i++) {
                sscanf(&hash_str[i*2], "%2hhx", &hash[i]);
            }
            repo->head = (Commit*)get_object(repo->store, hash);
        }
        fclose(f);
    }
    
    f = fopen(BRANCH_FILE, "r");
    if (f) {
        char branch[100];
        if (fgets(branch, 100, f)) {
            branch[strcspn(branch, "\n")] = 0;
            if (repo->current_branch) free(repo->current_branch);
            repo->current_branch = strdup(branch);
        }
        fclose(f);
    }
    
    // ✅ Загружаем staging area
    load_staging_area(repo);
    
    return 0;
}