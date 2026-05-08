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
    char hostname[100];
    char cwd[1024];
    
    while (1){
        gethostname(hostname, sizeof(hostname));
        getcwd(cwd, sizeof(cwd));

        char *path_min = strstr(cwd, "interpretador-so2");

        if (path_min == NULL) {
            path_min = cwd;
        }
        
        printf("[MySh] %s@%s: ~%s$ ", getenv("USER"), hostname, path_min);
        fgets(input, sizeof(input), stdin);
        clear_line(input);
        
        // COMANDO DE SAÍDA
        if (strcmp(input, "exit") == 0) {
            break;
        } else if (strcmp(input, "help") == 0) {
            printf("Available commands:\n");
            printf("help - Show this help message\n");
            printf("exit - Exit the shell\n");
            printf("clear - Clear the terminal\n");
            printf("listprogs - List available programs\n");
            clear_line(input);
            continue;
        }

        //COMANDO CD (Deslocamento entre diretórios)
        else if (strcmp(input, "cd") == 0 || strncmp(input, "cd ", 3) == 0) {
            char input_copy[100];
            strcpy(input_copy, input);

            strtok(input_copy, " ");
            char *path = strtok(NULL, " ");

            if (path == NULL) {
                path = getenv("HOME");
            }

            if (path == NULL) {
                printf("Erro: variável HOME não encontrada.\n");
                continue;
            }

            if (chdir(path) != 0) {
                perror("Erro ao mudar de diretório");
            }

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

            if (dir == NULL) {
                perror("Unable to open directory");
                continue;
            }

            printf("Available programs:\n");

            while ((entry = readdir(dir)) != NULL) {
                char *ext = strrchr(entry->d_name, '.');

                // Ignora arquivos ocultos, como . e ..
                if (entry->d_name[0] == '.') {
                    continue;
                }

                // Mostra apenas arquivos .c
                if (ext != NULL && strcmp(ext, ".c") == 0) {
                    char program_name[200];

                    strcpy(program_name, entry->d_name);

                    // Remove a extensão .c
                    char *dot = strrchr(program_name, '.');
                    if (dot != NULL) {
                        *dot = '\0';
                    }

                    printf("- %s\n", program_name);
                }
            }

            closedir(dir);
        }

        // EXECUÇÃO DE PROGRAMAS
        
        else {
            char *args[10];
            int i = 0;

            args[i] = strtok(input, " ");

            while (args[i] != NULL && i < 9) {
                i++;
                args[i] = strtok(NULL, " ");
            }

            args[9] = NULL;

            if (args[0] == NULL) {
                continue;
            }

            char source_path[200];
            char exec_path[200];

            sprintf(source_path, "../programs/%s.c", args[0]);
            sprintf(exec_path, "../programs/%s", args[0]);

            /*
                Verifica se existe um arquivo .c com o nome do comando.
                Exemplo:
                usuário digita "hello"
                shell procura "../programs/hello.c"
            */
            if (access(source_path, F_OK) == 0) {
                printf("Compilando e executando programa: %s.c...\n", args[0]);
                pid_t compile_pid = fork();

                if (compile_pid == 0) {
                    char *gcc_args[] = {"gcc", source_path, "-o", exec_path, NULL};

                    execvp("gcc", gcc_args);

                    perror("Erro ao executar gcc");
                    exit(1);
                } 
                else if (compile_pid > 0) {
                    int status;
                    waitpid(compile_pid, &status, 0);

                    /*macros para ver se o filho fez a compilação corretamente 
                    WIFEXITED: verifica se o filho terminou normalmente
                    WEXITSTATUS: pega o código de saída do filho*/

                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                        pid_t run_pid = fork();

                        if (run_pid == 0) {
                            execvp(exec_path, args);

                            perror("Erro ao executar programa compilado");
                            exit(1);
                        } 
                        else if (run_pid > 0) {
                            waitpid(run_pid, NULL, 0);
                        } 
                        else {
                            perror("Erro no fork da execução");
                        }
                    } 
                    else {
                        printf("Erro: falha na compilação de %s\n", source_path);
                    }
                } 
                else {
                    perror("Erro no fork da compilação");
                }
            } 
            else {
                pid_t pid = fork();

                if (pid == 0) {
                    execvp(args[0], args);

                    perror("Erro ao executar comando");
                    exit(1);
                } 
                else if (pid > 0) {
                    waitpid(pid, NULL, 0);
                } 
                else {
                    perror("Erro no fork");
                }
            }
        }
    
    }
}