#include <stdio.h>
#include <string.h>
#include "include/minigit.h"

// Декларации функций
void cmd_log(RepoState *repo);
void cmd_show(RepoState *repo, const char *hash);
void cmd_stats(RepoState *repo);
void cmd_ls_tree(RepoState *repo, const char *hash);
void cmd_ls_files(RepoState *repo);
void cmd_branch_list(RepoState *repo);
int cmd_create_branch(RepoState *repo, const char *name);
int cmd_switch_branch(RepoState *repo, const char *name);
int cmd_delete_branch(RepoState *repo, const char *name);
int cmd_add(RepoState *repo, const char *path, const char *content);
Commit* cmd_commit(RepoState *repo, const char *message);
RepoState* init_repo(void);

// Статическая переменная для отслеживания создания коммитов
static int test_commits_created = 0;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: minigit <command> [args]\n");
        printf("Commands:\n");
        printf("  add <file> <content>  - Add file to staging area\n");
        printf("  commit <message>      - Create a commit\n");
        printf("  log                   - Show commit history\n");
        printf("  show <hash>           - Show commit details\n");
        printf("  stats                 - Show repository statistics\n");
        printf("  ls-tree <hash>        - List files in a tree\n");
        printf("  ls-files              - List files in working directory\n");
        printf("  branch                - List branches\n");
        printf("  branch <name>         - Create a new branch\n");
        printf("  branch checkout <name> - Switch to branch\n");
        printf("  branch delete <name>  - Delete a branch\n");
        return 1;
    }
    
    RepoState *repo = init_repo();
    if (!repo) {
        printf("Failed to init repo\n");
        return 1;
    }
    // ВРЕМЕННО: создаем тестовые коммиты ТОЛЬКО если репозиторий пустой
    // и это первый запуск (закомментируй если не нужно)
    /*
    if (!repo->head) {
        printf("Creating test commits for demo...\n");
        
        // Создаем файлы
        Blob *readme = create_blob("# MiniGit Project\n\nA simple Git implementation");
        Blob *main_c = create_blob("#include <stdio.h>\n\nint main() {\n    printf(\"Hello\\n\");\n    return 0;\n}");
        
        // Первый коммит
        Tree *tree1 = create_tree();
        add_tree_entry(tree1, "README.md", 1, readme->hash);
        add_tree_entry(tree1, "main.c", 1, main_c->hash);
        Commit *c1 = create_commit(NULL, tree1, "Initial commit");
        
        // Второй коммит
        Blob *main_c_v2 = create_blob("#include <stdio.h>\n\nint main() {\n    printf(\"Hello MiniGit!\\n\");\n    return 0;\n}");
        Tree *tree2 = create_tree();
        add_tree_entry(tree2, "README.md", 1, readme->hash);
        add_tree_entry(tree2, "main.c", 1, main_c_v2->hash);
        Commit *c2 = create_commit(c1, tree2, "Add feature");
        
        // Третий коммит
        Blob *todo = create_blob("- Add commit command\n- Add branch support\n- Add checkout");
        Tree *tree3 = create_tree();
        add_tree_entry(tree3, "README.md", 1, readme->hash);
        add_tree_entry(tree3, "main.c", 1, main_c_v2->hash);
        add_tree_entry(tree3, "TODO.txt", 1, todo->hash);
        Commit *c3 = create_commit(c2, tree3, "Fix bug");
        
        repo->head = c3;
        cmd_create_branch(repo, "develop");
        
        printf("Created 3 commits and 'develop' branch\n");
    }
    */

    char *command = argv[1];
    
    if (strcmp(command, "add") == 0) {
        if (argc >= 4) {
            cmd_add(repo, argv[2], argv[3]);
        } else {
            printf("Usage: minigit add <filename> <content>\n");
        }
    }
    else if (strcmp(command, "commit") == 0) {
        if (argc >= 3) {
            cmd_commit(repo, argv[2]);
        } else {
            printf("Usage: minigit commit <message>\n");
        }
    }
    else if (strcmp(command, "log") == 0) {
        cmd_log(repo);
    }
    else if (strcmp(command, "stats") == 0) {
        cmd_stats(repo);
    }
    else if (strcmp(command, "ls-tree") == 0) {
        if (argc > 2) {
            cmd_ls_tree(repo, argv[2]);
        } else {
            printf("Usage: minigit ls-tree <hash>\n");
        }
    }
    else if (strcmp(command, "ls-files") == 0) {
        cmd_ls_files(repo);
    }
    else if (strcmp(command, "show") == 0) {
        if (argc > 2) {
            cmd_show(repo, argv[2]);
        } else {
            printf("Usage: minigit show <hash>\n");
        }
    }
    else if (strcmp(command, "branch") == 0) {
        if (argc == 2) {
            cmd_branch_list(repo);
        }
        else if (argc == 3) {
            cmd_create_branch(repo, argv[2]);
        }
        else if (argc == 4) {
            if (strcmp(argv[2], "checkout") == 0) {
                cmd_switch_branch(repo, argv[3]);
            }
            else if (strcmp(argv[2], "delete") == 0) {
                cmd_delete_branch(repo, argv[3]);
            }
            else {
                printf("Unknown branch command\n");
            }
        }
        else {
            printf("Unknown branch command\n");
        }
    }
    else {
        printf("Unknown command: %s\n", command);
    }
    
    return 0;
}