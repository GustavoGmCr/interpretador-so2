// Gustavo Frias

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* program_name;

void print_usage(FILE* stream, int exit_code)
{
    fprintf(stream, "Uso: %s [opcoes]\n", program_name);
    fprintf(stream,
        "  -h  --help              Mostra estas informacoes de uso.\n"
        "  -n  --numero valor      Define o valor inicial.\n"
        "  -a  --adicao valor      Realiza uma adicao.\n"
        "  -s  --subtracao valor   Realiza uma subtracao.\n"
        "  -m  --multiplicacao valor Realiza uma multiplicacao.\n"
        "  -d  --divisao valor     Realiza uma divisao.\n"
    );

    exit(exit_code);
}

int main(int argc, char* argv[])
{
    int next_option;
    int number = 0;

    program_name = argv[0];

    const char* const short_options = "hn:a:s:m:d:";

    const struct option long_options[] = {
        { "help",          0, NULL, 'h' },
        { "numero",        1, NULL, 'n' },
        { "adicao",        1, NULL, 'a' },
        { "subtracao",     1, NULL, 's' },
        { "multiplicacao", 1, NULL, 'm' },
        { "divisao",       1, NULL, 'd' },
        { NULL,            0, NULL,  0  }
    };

    do {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);

        switch (next_option)
        {
            case 'h':
                print_usage(stdout, 0);

            case 'n':
                number = atoi(optarg);
                break;

            case 'a':
                number += atoi(optarg);
                break;

            case 's':
                number -= atoi(optarg);
                break;

            case 'm':
                number *= atoi(optarg);
                break;

            case 'd':
                if (atoi(optarg) == 0) {
                    fprintf(stderr, "Erro: divisao por zero nao e permitida.\n");
                    exit(2);
                }

                number /= atoi(optarg);
                break;

            case '?':
                print_usage(stderr, 2);

            case -1:
                break;

            default:
                abort();
        }

    } while (next_option != -1);

    printf("Resultado: %d\n", number);

    return 0;
}