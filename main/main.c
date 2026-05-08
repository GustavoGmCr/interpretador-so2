#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

void clear_line(char *input) {
    input[strcspn(input, "\n")] = 0; 
}
void clear_terminal(){
    printf("\033[H\033[J");
}

int main() {
    char input[100];
    char cwd[1024];
    while (1){
        printf("[MySh] %s: ~%s$ ", getenv("USER"), getcwd(cwd, sizeof(cwd)));
        fgets(input, sizeof(input), stdin);
        clear_line(input);
        
        // COMANDO DE SAÍDA
        if (strcmp(input, "exit") == 0) {
            break;
            return 0;
        } else if (strcmp(input, "help") == 0) {
            printf("Available commands:\n");
            printf("help - Show this help message\n");
            printf("exit - Exit the shell\n");
            printf("clear - Clear the terminal\n");
            printf("listprogs - List available programs\n");
            clear_line(input);
            continue;
        }
        // COMANDO DE AJUDA
        else if (strcmp(input, "clear") == 0) {
            clear_terminal();
            continue;
        } 
        // COMANDO DE LISTAGEM DE PROGRAMAS
        else if (strcmp(input, "listprogs") == 0) {
            DIR *dir;
            struct dirent *entry;
            dir = opendir("../programs");
            while ((entry = readdir(dir)) != NULL) {
                char *ext = strrchr(entry->d_name, '.');
                if (entry->d_name[0] == '.' || ext != NULL && strcmp(ext, ".c") == 0)
                    continue;

                printf("- %s\n", entry->d_name);
            }
        } 
        // EXECUÇÃO DE PROGRAMAS
        else {
            char *args[10];
            int i = 0;
            args[i] = strtok(input, " ");
            while (args[i] != NULL) {
                i++;
                args[i] = strtok(NULL, " ");
            }
            pid_t pid = fork();

            if (pid == 0) {
                char path[200];
                sprintf(path, "../programs/%s", args[0]);
                if (access(path, X_OK) == 0) {
                    execvp(path, args);
                } else {
                    execvp(args[0], args);
                }
                perror("Error");
                clear_line(input);
                continue;
            }
            else {
                wait(NULL);
            }
        }
    }
}