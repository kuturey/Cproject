#include "minigit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int passed = 0;
int failed = 0;

void check(const char *test_name, int ok) {
    if (ok) {
        printf("  PASS %s\n", test_name);
        passed++;
    } else {
        printf("  FAIL %s\n", test_name);
        failed++;
    }
}

void test_hash(void) {
    printf("\n hash\n");

    unsigned char h1[SHA1_HASH_SIZE], h2[SHA1_HASH_SIZE], h3[SHA1_HASH_SIZE];
    compute_hash("hello", 5, h1);
    compute_hash("hello", 5, h2);
    compute_hash("world", 5, h3);

    check("одинаковый ввод даёт одинаковый хеш", memcmp(h1, h2, SHA1_HASH_SIZE) == 0);

    check("разный ввод даёт разный хеш", memcmp(h1, h3, SHA1_HASH_SIZE) != 0);

    unsigned char he1[SHA1_HASH_SIZE], he2[SHA1_HASH_SIZE];
    compute_hash("", 0, he1);
    compute_hash("", 0, he2);
    check("хеш пустой строки воспроизводим", memcmp(he1, he2, SHA1_HASH_SIZE) == 0);
}

void test_blob_sharing(void) {
    printf("\n blob\n");

    init_object_store();

    Blob *b1 = create_blob("hello");
    Blob *b2 = create_blob("hello"); 
    Blob *b3 = create_blob("world"); 

    check("блоб создаётся корректно", b1 != NULL && strcmp(b1->content, "hello") == 0);

    check("одинаковый контент — один объект", b1 == b2);

    check("разный контент — разные объекты с разными хешами", b1 != b3 && memcmp(b1->hash, b3->hash, SHA1_HASH_SIZE) != 0);

    ObjectStore *store = get_global_store();
    int before = store->total_count;
    create_blob("hello");
    check("дублирующий блоб не добавляется в store", store->total_count == before);
}

void test_tree(void) {
    printf("\n tree\n");

    unsigned char h1[SHA1_HASH_SIZE], h2[SHA1_HASH_SIZE];
    memset(h1, 0xAB, SHA1_HASH_SIZE);
    memset(h2, 0xCD, SHA1_HASH_SIZE);

    Tree *t = create_tree();
    check("новое дерево создаётся и оно пустое", t != NULL && t->entry_count == 0);

    add_tree_entry(t, "file.txt", 1, h1);
    check("файл добавляется и находится через функцию нахождения файла", t->entry_count == 1 && find_tree_entry(t, "file.txt") != NULL);

    check("find_tree_entry возвращает NULL для несуществующего файла", find_tree_entry(t, "ghost.txt") == NULL);

    add_tree_entry(t, "file.txt", 1, h2); // обновление 
    TreeEntry *e = find_tree_entry(t, "file.txt");
    check("обновление файла не дублирует запись, хеш меняется",
          t->entry_count == 1 && e != NULL &&
          memcmp(e->hash, h2, SHA1_HASH_SIZE) == 0);

    add_tree_entry(t, "readme.md", 1, h1);
    check("можно добавить несколько файлов", t->entry_count == 2);

    check("remove_tree_entry удаляет файл и возвращает 1",
          remove_tree_entry(t, "file.txt") == 1 &&
          find_tree_entry(t, "file.txt") == NULL);

    check("после удаления одного файла второй остался", find_tree_entry(t, "readme.md") != NULL);

    check("удаление несуществующего файла возвращает 0", remove_tree_entry(t, "ghost.txt") == 0);

    free_tree(t);
}

void test_tree_hash_and_clone(void) {
    printf("\n tree hash + clone_tree\n");

    unsigned char h1[SHA1_HASH_SIZE], h2[SHA1_HASH_SIZE];
    memset(h1, 0x11, SHA1_HASH_SIZE);
    memset(h2, 0x22, SHA1_HASH_SIZE);

    Tree *t1 = create_tree();
    Tree *t2 = create_tree();
    add_tree_entry(t1, "a.txt", 1, h1);
    add_tree_entry(t2, "a.txt", 1, h1);
    check("деревья с одинаковым содержимым равны", tree_equals(t1, t2));

    add_tree_entry(t2, "a.txt", 1, h2);
    check("изменение содержимого меняет хеш дерева", !tree_equals(t1, t2));

    Tree *clone = clone_tree(t1);
    check("Копирование дерева создаёт отдельный объект с тем же хешем", clone != NULL && clone != t1 && tree_equals(t1, clone));

    free_tree(t1);
    free_tree(t2);
    free_tree(clone);
}

void test_object_store(void) {
    printf("\n object_store\n");

    ObjectStore *store = get_global_store();

    unsigned char key[SHA1_HASH_SIZE];
    memset(key, 0x77, SHA1_HASH_SIZE);

    Blob *blob = (Blob*)calloc(1, sizeof(Blob));
    blob->type = OBJ_BLOB;
    memcpy(blob->hash, key, SHA1_HASH_SIZE);

    int before = store->total_count;
    add_object(store, blob, key);
    check("add_object увеличивает общее количество на 1", store->total_count == before + 1);

    check("get_object возвращает тот же указатель", get_object(store, key) == blob);

    check("has_object находит добавленный объект", has_object(store, key));

    add_object(store, blob, key); /* дубликат */
    check("повторное добавление не увеличивает общее количество", store->total_count == before + 1);

    free(blob);
}

void test_commit_and_persistence(void) {
    printf("\n persistence\n");

    ObjectStore *store = get_global_store();

    Blob *bv1 = create_blob("version 1");
    Tree *tree1 = create_tree();
    add_tree_entry(tree1, "doc.txt", 1, bv1->hash);
    add_object(store, tree1, tree1->hash);
    Commit *c1 = create_commit(NULL, tree1, "first commit");

    check("начальный коммит создаётся с правильным сообщением",
          c1 != NULL && strcmp(c1->message, "first commit") == 0);

    unsigned char zero[SHA1_HASH_SIZE];
    memset(zero, 0, SHA1_HASH_SIZE);
    check("у начального коммита родительский хэш нулевой", memcmp(c1->parent_hash, zero, SHA1_HASH_SIZE) == 0);

    check("у начального коммита нет родителя в хранилище", get_parent_commit(c1, store) == NULL);

    /* Копирование хеша дерева c1 для проверки персистентности */
    unsigned char copy[SHA1_HASH_SIZE];
    memcpy(copy, c1->tree_hash, SHA1_HASH_SIZE);

    Blob *bv2 = create_blob("version 2");
    Tree *tree2 = clone_tree(tree1);
    add_tree_entry(tree2, "doc.txt", 1, bv2->hash);
    add_object(store, tree2, tree2->hash);
    Commit *c2 = create_commit(c1, tree2, "second commit");

    check("второй коммит ссылается на первый через родительский хэш",
          memcmp(c2->parent_hash, c1->hash, SHA1_HASH_SIZE) == 0);

    check("хеши разных коммитов различаются",
          memcmp(c1->hash, c2->hash, SHA1_HASH_SIZE) != 0);

    check("хэш дерева первого коммита не изменился после создания второго",
          memcmp(c1->tree_hash, copy, SHA1_HASH_SIZE) == 0);

    char *content1 = get_file_content(c1, "doc.txt", store);
    check("из первого коммита читается старое содержимое файла",
          content1 != NULL && strcmp(content1, "version 1") == 0);
    free(content1);

    char *content2 = get_file_content(c2, "doc.txt", store);
    check("из второго коммита читается новое содержимое файла",
          content2 != NULL && strcmp(content2, "version 2") == 0);
    free(content2);

    check("get_file_exists: существующий файл в коммите",
          get_file_exists(c1, "doc.txt", store) == 1);

    check("get_file_exists и get_file_content для несуществующего файла",
          get_file_exists(c1, "missing.txt", store) == 0 &&
          get_file_content(c1, "missing.txt", store) == NULL);

    free_tree(tree1);
    free_tree(tree2);
}

void clear_branches(void) {
    extern BranchNode *branches;
    BranchNode *cur = branches;
    while (cur) { BranchNode *next = cur->next; free(cur); cur = next; }
    branches = NULL;
}

void test_branches(void) {
    printf("\n branches\n");

    clear_branches();

    ObjectStore *store = get_global_store();
    Tree *t = create_tree();
    add_object(store, t, t->hash);
    Commit *c1 = create_commit(NULL, t, "init");
    Commit *c2 = create_commit(c1, t, "second");

    RepoState repo;
    repo.head = c1;
    repo.current_branch = stringdup("master");
    repo.staging_area = create_tree();
    repo.store = store;
    repo.path = NULL;

    init_branches(&repo);
    check("init_branchs создаёт ветку master", find_branch("master") != NULL);

    check("cmd_create_branch создаёт ветку и возвращает 0",
          cmd_create_branch(&repo, "feature") == 0 &&
          find_branch("feature") != NULL);

    check("повторное создание той же ветки возвращает -1", cmd_create_branch(&repo, "feature") == -1);

    BranchNode *fb = find_branch("feature");
    if (fb) memcpy(fb->commit_hash, c2->hash, SHA1_HASH_SIZE);

    check("cmd_switch_branch переключает ветку и возвращает 0",
          cmd_switch_branch(&repo, "feature") == 0 &&
          strcmp(repo.current_branch, "feature") == 0);

    check("переключение на несуществующую ветку возвращает -1",
          cmd_switch_branch(&repo, "nonexistent") == -1);

    cmd_switch_branch(&repo, "master");
    check("cmd_delete_branch удаляет ветку и возвращает 0",
          cmd_delete_branch(&repo, "feature") == 0 &&
          find_branch("feature") == NULL);

    check("нельзя удалить текущую ветку", cmd_delete_branch(&repo, "master") == -1);

    free(repo.current_branch);
    free_tree(repo.staging_area);
    free_tree(t);
    clear_branches();
}

int main(void){

    test_hash();
    test_blob_sharing();
    test_tree();
    test_tree_hash_and_clone();
    test_object_store();
    test_commit_and_persistence();
    test_branches();

    printf("passed: %d  failed: %d\n", passed, failed);

    return failed == 0 ? 0 : 1;
}
