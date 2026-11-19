#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "meu_banco.h"

int main() {
    Banco banco;
    char linha[16];
    char op;

    iniciarBanco(&banco);

    do {
        printf("\n===== Visao do Banco =====\n");
        printf("[a] Abrir o banco\n");
        printf("[h] Consultar historico\n");
        printf("[e] Efetuar sorteio\n");
        printf("[s] Sair\n");
        printf("Opcao: ");
        if (lerLinha(linha, 16) == 0) {
            break;
        }
        if (strlen(linha) == 1) {
            op = tolower((unsigned char)linha[0]);
        } else {
            op = '?';
        }

        if (op == 'a') {
            abrirBanco(&banco);
        } else if (op == 'h') {
            historicoBanco(&banco);
        } else if (op == 'e') {
            efetuarSorteio(&banco);
        } else if (op == 's') {
            printf("Encerrando...\n");
        } else {
            printf("Opcao invalida.\n");
        }
    } while (op != 's');

    if (banco.aberto) {
        salvarBanco(&banco);
    }
    liberarBanco(&banco);
    return 0;
}
