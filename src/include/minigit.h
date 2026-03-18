#ifndef MINIGIT_H
#define MINIGIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SHA1_HASH_SIZE 20
#define MAX_PATH 260
#define MAX_MESSAGE 1024

// ============ Типы объектов ============
typedef enum {
    OBJ_BLOB = 0,
    OBJ_TREE = 1,
    OBJ_COMMIT = 2
} ObjectType;

// ============ Основные структуры данных ============

typedef struct Blob {
    ObjectType type;                    // <-- ДОБАВЛЕНО
    unsigned char hash[SHA1_HASH_SIZE];
    char *content;
    size_t size;
} Blob;

typedef struct TreeEntry {
    char *name;
    enum { BLOB_ENTRY, TREE_ENTRY } type;  // тип записи (не путать с ObjectType!)
    unsigned char hash[SHA1_HASH_SIZE];
} TreeEntry;

typedef struct Tree {
    ObjectType type;                    // <-- ДОБАВЛЕНО
    unsigned char hash[SHA1_HASH_SIZE];
    TreeEntry *entries;
    int entry_count;
} Tree;

typedef struct Commit {
    ObjectType type;                    // <-- ДОБАВЛЕНО
    unsigned char hash[SHA1_HASH_SIZE];
    unsigned char tree_hash[SHA1_HASH_SIZE];
    unsigned char parent_hash[SHA1_HASH_SIZE];
    char *message;
    time_t timestamp;
} Commit;

// ============ Хеш-таблица для Object Store ============
typedef struct HashBucket {
    void *object;
    unsigned char hash[SHA1_HASH_SIZE];
    struct HashBucket *next;
} HashBucket;

typedef struct ObjectStore {
    HashBucket *buckets[256];
    int bucket_count[256];
    int total_count;
} ObjectStore;

// ============ Состояние репозитория ============
typedef struct RepoState {
    Commit *head;
    char *current_branch;
    Tree *staging_area;
    ObjectStore *store;
    char *path;  // путь к репозиторию (для сохранения)
} RepoState;

// ============ Ветка ============
typedef struct BranchNode {
    char name[100];
    unsigned char commit_hash[SHA1_HASH_SIZE];
    struct BranchNode *next;
} BranchNode;

// ============ Прототипы функций ============

// blob.c
Blob* create_blob(const char *content);
void free_blob(Blob *blob);

// tree.c
Tree* create_tree(void);
void free_tree(Tree *tree);
void add_tree_entry(Tree *tree, const char *name, int is_blob, const unsigned char *hash);
TreeEntry* find_tree_entry(Tree *tree, const char *name);
void print_tree(Tree *tree);

// commit.c
Commit* create_commit(Commit *parent, Tree *root, const char *message);
void free_commit(Commit *commit);
Commit* get_parent_commit(Commit *commit, ObjectStore *store);
void print_commit(Commit *commit);
void print_history(Commit *start, ObjectStore *store);

// object_store.c
ObjectStore* init_object_store(void);
void free_object_store(ObjectStore *store);
void add_object(ObjectStore *store, void *obj, const unsigned char *hash);
void* get_object(ObjectStore *store, const unsigned char *hash);
ObjectStore* get_global_store(void);
void print_store_stats(ObjectStore *store);
int has_object(ObjectStore *store, const unsigned char *hash);

// hash.c
void compute_hash(const void *data, size_t len, unsigned char *hash);

// ============ Функции команд ============
int cmd_add(RepoState *repo, const char *path, const char *content);
void print_staging_area(RepoState *repo);

Commit* cmd_commit(RepoState *repo, const char *message);
void cmd_status(RepoState *repo);

void cmd_log(RepoState *repo);
void cmd_show(RepoState *repo, const char *commit_hash);

int cmd_checkout(RepoState *repo, const char *target);

int cmd_create_branch(RepoState *repo, const char *name);
int cmd_switch_branch(RepoState *repo, const char *name);
void cmd_branch_list(RepoState *repo);
int cmd_delete_branch(RepoState *repo, const char *name);
BranchNode* find_branch(const char *name);

void cmd_stats(RepoState *repo);
void cmd_ls_tree(RepoState *repo, const char *hash);
void cmd_ls_files(RepoState *repo);

// repo.c
RepoState* init_repo(void);
void free_repo(RepoState *repo);
void set_head(RepoState *repo, Commit *commit);

// save.c
// Функции из save.c
void load_all_objects_from_disk(ObjectStore *store, const char *objects_dir);
void save_branches_to_disk(void);
void load_branches_from_disk(RepoState *repo);
void init_branches(RepoState *repo);
int save_repo_state(RepoState *repo);
int load_repo_state(RepoState *repo);
void save_object(ObjectStore *store, void *obj, ObjectType type, const unsigned char *hash);  // <-- изменён прототип
void* load_object(ObjectStore *store, const unsigned char *hash);
// Объявление глобальной переменной branches (определена в cmd_branch_list.c)
extern BranchNode *branches;
#endif // MINIGIT_H