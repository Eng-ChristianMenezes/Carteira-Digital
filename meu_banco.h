#ifndef MEU_BANCO_H
#define MEU_BANCO_H

typedef struct {
    int existe;
    char numero[16];
    float saldo;
    float histValor[5];
    char histOp[5];
    int qtdHist;
} Conta;

typedef struct {
    char nome[50];
    Conta contas[3];
    int emprestimo;
    int titulo;
} Cliente;

typedef struct {
    Cliente *clientes;
    int qtd;
    int aberto;
    int proximo;
    int sorteado;
    char ganhador[50];
    char numeroGanhador[16];
    float premio;
} Banco;

void iniciarBanco(Banco *b);
void liberarBanco(Banco *b);
void abrirBanco(Banco *b);
void historicoBanco(Banco *b);
void efetuarSorteio(Banco *b);
void salvarBanco(Banco *b);
int lerLinha(char *buf, int tam);
void limparEspacos(char *s);

#endif
