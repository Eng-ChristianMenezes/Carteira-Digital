#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include "meu_banco.h"

Banco *bancoAtual = NULL;

void limparEspacos(char *s) {
    int ini = 0;
    int fim = strlen(s);

    while (fim > 0 && isspace((unsigned char)s[fim - 1])) {
        fim--;
    }
    s[fim] = '\0';
    while (s[ini] != '\0' && isspace((unsigned char)s[ini])) {
        ini++;
    }
    memmove(s, s + ini, strlen(s + ini) + 1);
}

int lerLinha(char *buf, int tam) {
    int c, n = 0;

    c = getchar();
    if (c == EOF) {
        return 0;
    }
    while (c != '\n' && c != EOF) {
        if (n < tam - 1) {
            buf[n] = c;
            n++;
        }
        c = getchar();
    }
    buf[n] = '\0';
    limparEspacos(buf);
    return 1;
}

void lerTexto(char *buf, int tam) {
    if (lerLinha(buf, tam) == 0) {
        printf("\nFim da entrada. Salvando e encerrando.\n");
        if (bancoAtual != NULL && bancoAtual->aberto) {
            salvarBanco(bancoAtual);
        }
        exit(0);
    }
}

int lerInteiro() {
    char linha[64];
    char *fim;
    long n;

    while (1) {
        lerTexto(linha, 64);
        errno = 0;
        n = strtol(linha, &fim, 10);
        if (linha[0] != '\0' && *fim == '\0' && errno == 0 && n >= INT_MIN && n <= INT_MAX) {
            return n;
        }
        printf("Valor invalido, digite de novo: ");
    }
}

float lerReal() {
    char linha[64];
    char *fim;
    double n;
    int i;

    while (1) {
        lerTexto(linha, 64);
        for (i = 0; linha[i] != '\0'; i++) {
            if (linha[i] == ',') {
                linha[i] = '.';
            }
        }
        errno = 0;
        n = strtod(linha, &fim);
        if (linha[0] != '\0' && *fim == '\0' && errno == 0 && isfinite(n) && n >= -1000000000 && n <= 1000000000) {
            return n;
        }
        printf("Valor invalido, digite de novo: ");
    }
}

void iniciarBanco(Banco *b) {
    b->clientes = NULL;
    b->qtd = 0;
    b->aberto = 0;
    b->proximo = 1000;
    b->sorteado = 0;
    strcpy(b->ganhador, "-");
    strcpy(b->numeroGanhador, "-");
    b->premio = 0;
    bancoAtual = b;
}

void liberarBanco(Banco *b) {
    free(b->clientes);
    b->clientes = NULL;
    b->qtd = 0;
}

float saldoTotal(Cliente *c) {
    float soma = 0;
    int i;

    for (i = 0; i < 3; i++) {
        if (c->contas[i].existe) {
            soma = soma + c->contas[i].saldo;
        }
    }
    return soma;
}

float limiteEmprestimo(Cliente *c) {
    return 0.25 * 10 * saldoTotal(c);
}

int podePedirEmprestimo(Cliente *c) {
    if (c->emprestimo == 1) {
        return 0;
    }
    if (c->contas[0].existe == 0 || c->contas[2].existe == 0) {
        return 0;
    }
    if (c->contas[2].saldo <= 0) {
        return 0;
    }
    return limiteEmprestimo(c) > 0;
}

int podeComprarTitulo(Cliente *c) {
    if (c->titulo == 1 || c->contas[1].existe == 0) {
        return 0;
    }
    return c->contas[1].saldo >= 1000 && c->contas[1].saldo <= 1500;
}

void registrar(Conta *conta, char op, float valor) {
    int i;

    if (conta->qtdHist == 5) {
        for (i = 1; i < 5; i++) {
            conta->histValor[i - 1] = conta->histValor[i];
            conta->histOp[i - 1] = conta->histOp[i];
        }
        conta->qtdHist = 4;
    }
    conta->histValor[conta->qtdHist] = valor;
    conta->histOp[conta->qtdHist] = op;
    conta->qtdHist++;
}

int buscarCliente(Banco *b, char *nome) {
    int i;

    for (i = 0; i < b->qtd; i++) {
        if (strcmp(b->clientes[i].nome, nome) == 0) {
            return i;
        }
    }
    return -1;
}

void salvarBanco(Banco *b) {
    FILE *arq = fopen("banco.txt", "w");
    int i, j, k;
    Cliente *c;
    Conta *conta;

    if (arq == NULL) {
        printf("Erro ao salvar o arquivo.\n");
        return;
    }
    fprintf(arq, "%d %d %s;%s;%.2f\n", b->proximo, b->sorteado, b->ganhador, b->numeroGanhador, b->premio);
    for (i = 0; i < b->qtd; i++) {
        c = &b->clientes[i];
        fprintf(arq, "%s;%d %d", c->nome, c->emprestimo, c->titulo);
        for (j = 0; j < 3; j++) {
            conta = &c->contas[j];
            if (conta->existe) {
                fprintf(arq, " 1 %s %.2f %d", conta->numero, conta->saldo, conta->qtdHist);
            } else {
                fprintf(arq, " 0 - 0 0");
            }
            for (k = 0; k < conta->qtdHist; k++) {
                fprintf(arq, " %c %.2f", conta->histOp[k], conta->histValor[k]);
            }
        }
        fprintf(arq, "\n");
    }
    fclose(arq);
}

void adicionarCliente(Banco *b, Cliente c) {
    Cliente *temp = realloc(b->clientes, (b->qtd + 1) * sizeof(Cliente));

    if (temp == NULL) {
        printf("Sem memoria! Salvando e encerrando.\n");
        salvarBanco(b);
        exit(1);
    }
    b->clientes = temp;
    b->clientes[b->qtd] = c;
    b->qtd++;
}

void copiarArquivo(char *origem, char *destino) {
    FILE *a = fopen(origem, "r");
    FILE *d = fopen(destino, "w");
    int c;

    if (a != NULL && d != NULL) {
        while ((c = fgetc(a)) != EOF) {
            fputc(c, d);
        }
    }
    if (a != NULL) {
        fclose(a);
    }
    if (d != NULL) {
        fclose(d);
    }
}

int lerClienteDaLinha(char *linha, Cliente *c) {
    int pos = 0, lidos, j, k;
    char sobra;
    Conta *conta;

    memset(c, 0, sizeof(Cliente));
    if (sscanf(linha, "%49[^;];%d %d%n", c->nome, &c->emprestimo, &c->titulo, &pos) != 3) {
        return 0;
    }
    limparEspacos(c->nome);
    if (c->nome[0] == '\0') {
        return 0;
    }
    if (c->emprestimo < 0 || c->emprestimo > 1 || c->titulo < 0 || c->titulo > 1) {
        return 0;
    }
    for (j = 0; j < 3; j++) {
        conta = &c->contas[j];
        lidos = 0;
        if (sscanf(linha + pos, " %d %15s %f %d%n", &conta->existe, conta->numero, &conta->saldo, &conta->qtdHist, &lidos) != 4) {
            return 0;
        }
        pos = pos + lidos;
        if (conta->existe < 0 || conta->existe > 1 || !isfinite(conta->saldo)) {
            return 0;
        }
        if (conta->qtdHist < 0 || conta->qtdHist > 5) {
            return 0;
        }
        if (conta->existe == 0 && conta->qtdHist != 0) {
            return 0;
        }
        for (k = 0; k < conta->qtdHist; k++) {
            lidos = 0;
            if (sscanf(linha + pos, " %c %f%n", &conta->histOp[k], &conta->histValor[k], &lidos) != 2) {
                return 0;
            }
            pos = pos + lidos;
            if (conta->histOp[k] != 'D' && conta->histOp[k] != 'S' && conta->histOp[k] != 'E') {
                return 0;
            }
        }
    }
    if (sscanf(linha + pos, " %c", &sobra) == 1) {
        return 0;
    }
    return 1;
}

void carregarBanco(Banco *b) {
    FILE *arq = fopen("banco.txt", "r");
    char linha[2048];
    char gan[50], num[16];
    Cliente c;
    int prox, sort;
    int invalido = 0;
    float prem;

    if (arq == NULL) {
        printf("Nenhum arquivo anterior. Banco novo.\n");
        return;
    }

    if (fgets(linha, sizeof(linha), arq) != NULL) {
        limparEspacos(linha);
        if (sscanf(linha, "%d %d %49[^;];%15[^;];%f", &prox, &sort, gan, num, &prem) == 5 && prox >= 1000 && prox <= 10000 && (sort == 0 || sort == 1)) {
            b->proximo = prox;
            b->sorteado = sort;
            strcpy(b->ganhador, gan);
            strcpy(b->numeroGanhador, num);
            b->premio = prem;
        } else {
            fclose(arq);
            copiarArquivo("banco.txt", "banco.bak");
            printf("AVISO: banco.txt invalido. Copia guardada em banco.bak. Banco novo.\n");
            return;
        }
    }

    while (fgets(linha, sizeof(linha), arq) != NULL) {
        limparEspacos(linha);
        if (linha[0] == '\0') {
            continue;
        }
        if (lerClienteDaLinha(linha, &c)) {
            adicionarCliente(b, c);
        } else {
            invalido = 1;
        }
    }
    fclose(arq);

    if (invalido) {
        copiarArquivo("banco.txt", "banco.bak");
        printf("AVISO: linhas invalidas no banco.txt foram ignoradas. Copia guardada em banco.bak.\n");
    }
    printf("%d cliente(s) carregado(s).\n", b->qtd);
}

void cadastrarClientes(Banco *b) {
    int n, i, tipo, terminou, ok;
    Cliente c;

    printf("\nQuantos clientes deseja cadastrar? (0 = nenhum): ");
    n = lerInteiro();

    for (i = 0; i < n; i++) {
        if (b->proximo > 9999) {
            printf("O banco nao tem mais numeros de conta disponiveis.\n");
            return;
        }
        memset(&c, 0, sizeof(Cliente));

        printf("\n--- Cliente %d ---\n", i + 1);
        do {
            printf("Nome do cliente: ");
            lerTexto(c.nome, 50);
            ok = 1;
            if (c.nome[0] == '\0') {
                printf("O nome nao pode ser vazio.\n");
                ok = 0;
            } else if (strchr(c.nome, ';') != NULL) {
                printf("O nome nao pode ter ponto e virgula.\n");
                ok = 0;
            } else if (buscarCliente(b, c.nome) != -1) {
                printf("Ja existe um cliente com esse nome.\n");
                ok = 0;
            }
        } while (ok == 0);

        terminou = 0;
        while (terminou == 0) {
            printf("Abrir conta (1-corrente 2-salario 3-poupanca 0-terminar): ");
            tipo = lerInteiro();
            if (tipo == 0) {
                if (c.contas[0].existe || c.contas[1].existe || c.contas[2].existe) {
                    terminou = 1;
                } else {
                    printf("O cliente precisa de pelo menos uma conta.\n");
                }
            } else if (tipo < 1 || tipo > 3) {
                printf("Tipo invalido.\n");
            } else if (c.contas[tipo - 1].existe) {
                printf("O cliente ja tem esse tipo de conta.\n");
            } else if ((tipo == 1 && c.contas[1].existe) || (tipo == 2 && c.contas[0].existe)) {
                printf("Nao pode ter conta salario e conta corrente juntas.\n");
            } else if (b->proximo > 9999) {
                printf("O banco nao tem mais numeros de conta disponiveis.\n");
                terminou = 1;
            } else {
                c.contas[tipo - 1].existe = 1;
                sprintf(c.contas[tipo - 1].numero, "%d-%02d", b->proximo, tipo);
                b->proximo++;
                printf("Conta criada: %s\n", c.contas[tipo - 1].numero);
            }
        }
        adicionarCliente(b, c);
    }
}

void menuCliente(Cliente *c, int j) {
    Conta *conta = &c->contas[j];
    int op = 0, k;
    float valor;

    printf("\nBem-vindo(a), %s!\n", c->nome);
    if (c->contas[0].existe && c->contas[0].saldo < 0) {
        printf("ALERTA: sua conta corrente esta negativa (R$ %.2f)\n", c->contas[0].saldo);
    }

    do {
        printf("\n1 - Depositar\n");
        printf("2 - Sacar\n");
        printf("3 - Solicitar emprestimo\n");
        printf("4 - Adquirir titulos de capitalizacao\n");
        printf("5 - Solicitar extrato\n");
        printf("0 - Voltar\n");
        printf("Opcao: ");
        op = lerInteiro();

        if (op == 1) {
            printf("Valor do deposito: R$ ");
            valor = lerReal();
            if (valor <= 0) {
                printf("Valor invalido.\n");
            } else {
                conta->saldo = conta->saldo + valor;
                registrar(conta, 'D', valor);
            }
        } else if (op == 2) {
            printf("Valor do saque: R$ ");
            valor = lerReal();
            if (valor <= 0) {
                printf("Valor invalido.\n");
            } else if (j != 0 && valor > conta->saldo) {
                printf("Saldo insuficiente.\n");
            } else {
                conta->saldo = conta->saldo - valor;
                registrar(conta, 'S', valor);
            }
        } else if (op == 3) {
            if (podePedirEmprestimo(c) == 0) {
                printf("Voce nao pode pedir emprestimo (precisa de corrente e poupanca com saldo, e so 1 por cliente).\n");
            } else {
                printf("Limite: R$ %.2f\n", limiteEmprestimo(c));
                printf("Valor do emprestimo: R$ ");
                valor = lerReal();
                if (valor <= 0 || valor > limiteEmprestimo(c)) {
                    printf("Valor invalido.\n");
                } else {
                    c->contas[0].saldo = c->contas[0].saldo + valor;
                    c->emprestimo = 1;
                    registrar(&c->contas[0], 'E', valor);
                    printf("Emprestimo creditado na conta corrente.\n");
                }
            }
        } else if (op == 4) {
            if (podeComprarTitulo(c)) {
                c->titulo = 1;
                printf("Titulo adquirido!\n");
            } else {
                printf("Nao pode comprar (conta salario com saldo entre 1000 e 1500, so 1 titulo).\n");
            }
        } else if (op == 5) {
            printf("\n== Extrato %s ==\n", conta->numero);
            for (k = 0; k < conta->qtdHist; k++) {
                printf("%.2f : %c\n", conta->histValor[k], conta->histOp[k]);
            }
            printf("Saldo atual: R$ %.2f\n", conta->saldo);
        } else if (op != 0) {
            printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

void acessoCliente(Banco *b) {
    char numero[64];
    int i, j, achou;

    do {
        printf("\nNumero da conta (0 para voltar): ");
        lerTexto(numero, 64);
        achou = 0;
        for (i = 0; i < b->qtd; i++) {
            for (j = 0; j < 3; j++) {
                if (b->clientes[i].contas[j].existe && strcmp(b->clientes[i].contas[j].numero, numero) == 0) {
                    menuCliente(&b->clientes[i], j);
                    achou = 1;
                }
            }
        }
        if (achou == 0 && strcmp(numero, "0") != 0) {
            printf("Conta nao encontrada.\n");
        }
    } while (strcmp(numero, "0") != 0);
}

void abrirBanco(Banco *b) {
    if (b->aberto == 0) {
        carregarBanco(b);
        b->aberto = 1;
    }
    printf("Banco aberto.\n");
    cadastrarClientes(b);
    acessoCliente(b);
    salvarBanco(b);
}

void historicoBanco(Banco *b) {
    int i;
    float total = 0;
    Cliente *c;

    if (b->aberto == 0) {
        printf("Abra o banco primeiro.\n");
        return;
    }

    for (i = 0; i < b->qtd; i++) {
        total = total + saldoTotal(&b->clientes[i]);
    }
    printf("\n== Saldo Total do banco ==\n");
    printf("%d | R$ %.2f\n", b->qtd, total);

    printf("\n== Clientes Aptos a Emprestimos ==\n");
    for (i = 0; i < b->qtd; i++) {
        c = &b->clientes[i];
        if (podePedirEmprestimo(c)) {
            printf("%s|%s|%.2f\n", c->nome, c->contas[0].numero, limiteEmprestimo(c));
        }
    }

    printf("\n== Clientes com Saldo Negativo na Conta Corrente ==\n");
    for (i = 0; i < b->qtd; i++) {
        c = &b->clientes[i];
        if (c->contas[0].existe && c->contas[0].saldo < 0) {
            printf("%s|%s|%.2f\n", c->nome, c->contas[0].numero, c->contas[0].saldo);
        }
    }

    printf("\n== Clientes Aptos a Compra de Titulos ==\n");
    for (i = 0; i < b->qtd; i++) {
        c = &b->clientes[i];
        if (podeComprarTitulo(c)) {
            printf("%s|%s|%.2f\n", c->nome, c->contas[1].numero, c->contas[1].saldo);
        }
    }

    printf("\n== Clientes Contemplado no Sorteio ==\n");
    if (b->sorteado == 1) {
        printf("%s|%s|%.2f\n", b->ganhador, b->numeroGanhador, b->premio);
    }
}

int fibonacci(int n) {
    if (n < 2) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int sortearPosicao(int qtd) {
    int n = 20 + time(NULL) % 10;
    return fibonacci(n) % qtd;
}

void efetuarSorteio(Banco *b) {
    int *ordem;
    int i, j, temp;
    Cliente *g;

    if (b->aberto == 0) {
        printf("Abra o banco primeiro.\n");
        return;
    }
    if (b->sorteado == 1) {
        printf("O sorteio ja foi feito.\n");
        return;
    }
    if (b->qtd < 5) {
        printf("Precisa de pelo menos 5 clientes.\n");
        return;
    }

    ordem = malloc(b->qtd * sizeof(int));
    if (ordem == NULL) {
        printf("Sem memoria!\n");
        return;
    }
    for (i = 0; i < b->qtd; i++) {
        ordem[i] = i;
    }
    for (i = 0; i < b->qtd - 1; i++) {
        for (j = 0; j < b->qtd - 1 - i; j++) {
            if (saldoTotal(&b->clientes[ordem[j]]) < saldoTotal(&b->clientes[ordem[j + 1]])) {
                temp = ordem[j];
                ordem[j] = ordem[j + 1];
                ordem[j + 1] = temp;
            }
        }
    }

    printf("\n== Os 5 clientes com maiores saldos ==\n");
    for (i = 0; i < 5; i++) {
        printf("%d. %s - R$ %.2f\n", i + 1, b->clientes[ordem[i]].nome, saldoTotal(&b->clientes[ordem[i]]));
    }
    for (i = 0; i < 5; i++) {
        if (b->clientes[ordem[i]].titulo == 0 || b->clientes[ordem[i]].contas[1].existe == 0) {
            printf("Sorteio cancelado: %s nao tem titulo.\n", b->clientes[ordem[i]].nome);
            free(ordem);
            return;
        }
    }

    g = &b->clientes[ordem[sortearPosicao(5)]];
    b->premio = 10 * g->contas[1].saldo;
    g->contas[1].saldo = g->contas[1].saldo + b->premio;
    registrar(&g->contas[1], 'D', b->premio);

    b->sorteado = 1;
    strcpy(b->ganhador, g->nome);
    strcpy(b->numeroGanhador, g->contas[1].numero);
    printf("\nContemplado: %s | conta %s | premio R$ %.2f\n", b->ganhador, b->numeroGanhador, b->premio);

    free(ordem);
    salvarBanco(b);
}
