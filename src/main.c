#include <stdio.h>
#include <string.h>
#include "include/minigit.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: minigit <command> [args]\n");
        printf("Commands:\n");
        printf("  add <file> <content>      - Add file to staging area\n");
        printf("  rm <file>                 - remove file in next version\n");
        printf("  commit <message>          - Create a commit\n");
        printf("  log                       - Show commit history\n");
        printf("  show <hash>               - Show commit details\n");
        printf("  stats                     - Show repository statistics\n");
        printf("  ls-tree <hash>            - List files in a tree\n");
        printf("  ls-files                  - List files in working directory\n");
        printf("  branch                    - List branches\n");
        printf("  branch head <name>        - show head commit of branch\n");
        printf("  branch <name>             - Create a new branch\n");
        printf("  branch checkout <name>    - Switch to branch\n");
        printf("  branch delete <name>      - Delete a branch\n");
        return 1;
    }
    
    RepoState *repo = init_repo();
    if (!repo) {
        printf("Failed to init repo\n");
        return 1;
    }

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
        else if (argc == 4 && strcmp(argv[2], "head") == 0) {
            Commit *head = get_branch_head(repo, argv[3]);

            if (!head) {
                printf("Branch or branch head not found: %s\n", argv[3]);
            } 
            else {
                printf("Branch %s head: ", argv[3]);
                printf(head->hash);
                printf("\n");
                print_commit(head);
            }
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