#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>


// guarda o pid do filho atual. Se não tiver filhos rodando, fica 0.
pid_t current_child = 0;

void clear_line(char *input) {
    input[strcspn(input, "\n")] = 0;  // pula linha e mostra o prompt vazio de novo, igual o terminal tradicional
}
void clear_terminal(){
    printf("\033[H\033[J");
}
void handle_sigint(int sig) { // funcao chamada no ctrl + c (signal interrupt)
    if (current_child != 0) {
        kill(current_child, SIGINT); //se tem filho rodando, encerra
    }
    write(STDOUT_FILENO, "\n", 1); // volta o terminal
}

int main() {
    char input[100];
    char hostname[100];
    char cwd[1024];

    struct sigaction sa; // struct de comportamento de sinais
    sa.sa_handler = handle_sigint; // atribui a funcao ao sinal sigint
    sigaction(SIGINT, &sa, NULL); // registra o comportamento para o sinal SIGINT (ctrl + c)
    
    while (1){
        gethostname(hostname, sizeof(hostname));
        getcwd(cwd, sizeof(cwd)); // pega o caminho

        char *path_min = strstr(cwd, "interpretador-so2"); // pega o caminho a partir da pasta do projeto
        if (path_min == NULL) {
            path_min = cwd; // se não encontrar a pasta do projeto, mostra o caminho completo
        }
        
        printf("[MySh] %s@%s:~%s$ ", getenv("USER"), hostname, path_min);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            continue;
        }
        clear_line(input);
        
        // COMANDO DE SAÍDA
        if (strcmp(input, "exit") == 0) {
            break;
        } else if (strcmp(input, "help") == 0) {
            printf("\n\n");
            printf("Available commands:\n");
            printf("help - Show this help message\n");
            printf("exit - Exit the shell\n");
            printf("clear - Clear the terminal\n");
            printf("printenv - Print environment variables\n");
            printf("create - Create a new C file\n");
            printf("edit - Edit an existing C file\n");
            printf("cd - Change directory\n");
            printf("listprogs - List available programs\n");  // VER SE VAI USAR 
            printf("\n\n");
            clear_line(input);
            continue;
        }

        //COMANDO CD (Deslocamento entre diretórios)

        // utilizando também strncmp para permitir cd acompanhado de ".."
        else if (strcmp(input, "cd") == 0 || strncmp(input, "cd ", 3) == 0) {
            char input_copy[100];
            strcpy(input_copy, input);
            strtok(input_copy, " ");
            char *path = strtok(NULL, " ");
            
            if (path == NULL) {
                printf("Erro: colocar caminho.\n");
                continue;
            }
            if (*path == '~') {
                path = getenv("HOME");
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
        
        // COMANDO PARA VISUALIZAR VARIÁVEIS DE AMBIENTE

        //Como nosso Interpretador é simples, vamos mostrar apenas 
        // as variáveis USER, HOST e PWD, que são as mais relevantes para o usuário.
        else if (strcmp(input, "printenv") == 0) {
            char hostname[100];
            char pwd[1024];
            char *user = getenv("USER");
            if (user == NULL) {
                user = "Unknown";
            }
            if(gethostname(hostname, sizeof(hostname)) != 0){
                strcpy(hostname, "Unknown");
            }
            if(getcwd(pwd, sizeof(pwd)) == NULL){
                strcpy(pwd, "Unknown");
            }
            printf("USER=%s\n", user);
            printf("HOST=%s\n", hostname);
            printf("PWD=%s\n", pwd);
            continue;
        }
        
        // COMANDO DE LISTAGEM DE PROGRAMAS
        else if (strcmp(input, "listprogs") == 0) {
            DIR *dir;
            struct dirent *entry;
            dir = opendir("./programs");//cria um fluxo de diretório para ler o conteúdo da pasta "programs"

            if (dir == NULL) {
                perror("Unable to open directory");
                continue;
            }
            printf("\n\n");
            printf("Available programs:\n");

            //vai ler o conteudo do diretorio que foi aberto opendir
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

                    // Remove a extensão .c oara deixar uma visualização melhor
                    char *dot = strrchr(program_name, '.');
                    if (dot != NULL) {
                        *dot = '\0';
                    }
                    printf("- %s\n", program_name);
                }
            }
            //fecha o fluxo de diretorio criado
            closedir(dir);
            printf("\n\n");
        }

        //COMANDO PARA CRIAR ARQUIVOS .c
        else if (strcmp(input, "create") == 0 || strncmp(input, "create ", 7) == 0) {
            char input_copy[100];
            strcpy(input_copy, input);
            strtok(input_copy, " ");

            // pode ser nome OU caminho
            char *filepath = strtok(NULL, " ");
            char *extra_arg = strtok(NULL, " ");

            if (filepath == NULL) {
                fprintf(stderr,"MySh: create: caminho do arquivo não informado\n");
                fprintf(stderr,"Uso correto: create <arquivo>\n");
                continue;
            }
            if (extra_arg != NULL) {
                fprintf(stderr,"MySh: create: argumentos demais\n");
                fprintf(stderr,"Uso correto: create <arquivo>\n");
                continue;
            }
            // verifica se já existe
            if (access(filepath, F_OK) == 0) { // access verifica se o arquivo existe, F_OK é a flag de verificação
                                               // pode-se ter R_OK e W_OK para ver permissão de read e write
                fprintf(stderr, "MySh: create: %s: arquivo já existe\n",filepath);
                continue;
            }
            FILE *file = fopen(filepath, "w");
            if (file == NULL) {
                fprintf(stderr, "MySh: create: %s: %s\n", filepath, strerror(errno));
                continue;
            }
            fclose(file);
            printf("Arquivo criado: %s\n", filepath);
            continue;
        }

        //COMANDO PARA EDITAR ARQUIVOS .c

        else if (strcmp(input, "edit") == 0 || strncmp(input, "edit ", 5) == 0) {
            char input_copy[100];
            strcpy(input_copy, input);
            strtok(input_copy, " ");
            // agora pode ser nome OU caminho
            char *filepath = strtok(NULL, " ");
            char *extra_arg = strtok(NULL, " ");
            if (filepath == NULL) {
                fprintf(stderr, "MySh: edit: caminho do arquivo não informado\n");
                fprintf(stderr, "Uso correto: edit <arquivo>\n");
                continue;
            }
            if (extra_arg != NULL) {
                fprintf(stderr, "MySh: edit: argumentos demais\n");
                fprintf(stderr, "Uso correto: edit <arquivo>\n");
                continue;
            }
            // verifica se o arquivo existe
            if (access(filepath, F_OK) != 0) {
                fprintf(stderr, "MySh: edit: %s: arquivo não encontrado\n", filepath);
                continue;
            }
            // pega editor padrão do sistema
            char *editor = getenv("EDITOR");
            if (editor == NULL) {
                editor = "nano";
            }
            pid_t editor_pid = fork();
            if (editor_pid == 0) {
                signal(SIGINT, SIG_DFL);
                char *editor_args[] = {
                    editor,
                    filepath,
                    NULL
                };
                execvp(editor, editor_args);
                fprintf(stderr, "MySh: %s: %s\n", editor, strerror(errno));
                exit(1);
            }
            else if (editor_pid > 0) {
                current_child = editor_pid;
                waitpid(editor_pid, NULL, 0);
                current_child = 0;
            }
            else {
                fprintf(stderr, "MySh: fork: %s\n", strerror(errno));
            }
            continue;
        }

        // EXECUÇÃO DE PROGRAMAS
        else {
            char *args[10];
            int i = 0;
            args[i] = strtok(input, " "); // pega o primeiro token de comando e depois os tokens de argumento
            while (args[i] != NULL && i < 9) { 
                i++;
                args[i] = strtok(NULL, " "); // pega os próximos tokens de argumento, se existirem, até o limite de 9 argumentos
            }
            args[9] = NULL;
            if (args[0] == NULL) {
                continue;
            }
            char source_path[200];
            char exec_path[200];
            sprintf(source_path, "%s.c", args[0]);
            sprintf(exec_path, "%s", args[0]);

            /*
                Verifica se existe um arquivo .c com o nome do comando.
                Exemplo:
                usuário digita "hello"
                shell procura "../programs/hello.c"
            */
            if (access(source_path, F_OK) == 0) { 
                printf("Compilando e executando programa: %s.c...\n", args[0]);
                pid_t compile_pid = fork(); // gera um processo filho para compilar o programa
                if (compile_pid == 0) {
                    char *gcc_args[] = {"gcc", source_path, "-o", exec_path, NULL}; // cria o comando gcc para compilar e passa pro execvp
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
                pid_t pid = fork(); // gera um processo filho
                if (pid == 0) {
                    signal(SIGINT, SIG_DFL); // restaura o comportamento padrão do sinal SIGINT para o processo filho
                    execvp(args[0], args); 

                    perror("Erro");
                    exit(1);
                }
                else if (pid > 0) {
                    current_child = pid; // pai guarda o pid do filho atual
                    waitpid(pid, NULL, 0); // espera o filho terminar
                    current_child = 0;
                }
                else {
                    perror("Erro no fork");
                }
            }
        }
    }
}