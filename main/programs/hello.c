#include <stdio.h>
#include <stdlib.h>

int main() {
    
    printf("\n\n");
    printf("Olá %s, fui compilado e executado pelo MySh!\n", getenv("USER"));
    printf("O MySh é um interpretador de comandos simples que suporta a execução de programas escritos em C.\n");
    printf("\n\n");
    
    return 0;
}