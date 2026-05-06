#include <stdio.h>
#include <string.h>
#include "include/minigit.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: minigit <command> [args]\n");
        printf("Commands:\n");
        printf("  init                          - Initialize empty repository\n");
        printf("  add <file> <content>          - Add file to staging area\n");
        printf("  rm <file>                     - remove file in next version\n");
        printf("  commit <message>              - Create a commit\n");
        printf("  print_history                 - Show commit history\n");
        printf("  print_commit <hash>           - Show commit details\n");
        printf("  content <commit|HEAD> <file>  - Get file content from version\n");
        printf("  exists <commit|HEAD> <file>   - Check file existence\n");
       // printf("  stats                     - Show repository statistics\n");
        printf("  print_files                   - List files in working directory\n");
        printf("  branch                        - List branches\n");
        printf("  get_branch_head <name>        - show head commit of branch\n");
        printf("  branch <name>                 - Create a new branch\n");
        printf("  checkout <name>               - Switch to commit\n");
        printf("  branch -d <name>              - Delete a branch\n");
        printf("  merge <branch> <message>      - merge: branch files over current without solving conflicts\n");
        return 1;
    }
    
    char *command = argv[1];
    
    RepoState *repo;

    if (strcmp(command, "init") == 0){
        repo = init_repo();
        if (!repo) {
            printf("Failed to init repo\n");
            return 1;
        }
    }
    else if (strcmp(command, "add") == 0) {
        if (argc >= 4) {
            cmd_add(repo, argv[2], argv[3]);
        } else {
            printf("Usage: minigit add <filename> <content>\n");
        }
    }
    else if (strcmp(command, "rm") == 0) {
        if (argv[2]) {
            cmd_remove(repo, argv[2]);
        } else {
            printf("Usage: minigit remove <filename>\n");
        }
    }
    else if (strcmp(command, "commit") == 0) {
        if (argc >= 3) {
            cmd_commit(repo, argv[2]);
        } else {
            printf("Usage: minigit commit <message>\n");
        }
    }
    else if (strcmp(command, "print_history") == 0) {
        cmd_log(repo);
    }
    /*else if (strcmp(command, "stats") == 0) {
        cmd_stats(repo);
    } переписать */
    else if (strcmp(command, "print_files") == 0) {
        cmd_print_files(repo->head, repo->store);
    }
    else if (strcmp(command, "print_commit") == 0) {
        if (argc > 2) {
            cmd_show(repo, argv[2]);
        } else {
            printf("Usage: minigit show <hash>\n");
        }
    }
    else if (strcmp(command, "content") == 0) {
        if (argc >= 4){ 
            cmd_content(repo, argv[2], argv[3]); 
        }
        else{ 
            printf("Usage: minigit cat <commit|HEAD> <file>\n");
        }
    }
    else if (strcmp(command, "exists") == 0) {
        if (argc >= 4){
            cmd_exists(repo, argv[2], argv[3]);
        }
        else{
            printf("Usage: minigit exists <commit|HEAD> <file>\n");
        }
    }
    else if (strcmp(command, "get_branch_head") == 0) {
        Commit *head = get_branch_head(repo, argv[2]);

        if (!head) {
            printf("Branch or branch head not found: %s\n", argv[2]);
        } 
        else {
            printf("Branch %s head: ", argv[2]);
            printf(head->hash);
            printf("\n");
            print_commit(head);
        }
    }
    else if (strcmp(command, "checkout") == 0) {
            cmd_checkout(repo, argv[2]);
        }
    else if (strcmp(command, "branch") == 0) {
        if (argc == 2) {
            cmd_branch_list(repo);
        }
        else if (argc == 3) {
            cmd_create_branch(repo, argv[2]);
        }
        else if (argc == 4) {
            if (strcmp(argv[2], "-d") == 0) {
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
    else if (strcmp(command, "merge") == 0) {
        if (argc >= 3){
            cmd_simple_merge(repo, argv[2], argc >= 4 ? argv[3] : "simple merge"); 
        }
        else{
            printf("Usage: minigit merge <branch> <message>\n");
        }
    }
    else {
        printf("Unknown command: %s\n", command);
    }
    
    return 0;
}