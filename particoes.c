#include "funcionarios.h"
#include <stdlib.h>
#include <limits.h>
#include "particoes.h"


void classificacao_interna(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc) {
    rewind(arq); //posiciona cursor no inicio do arquivo


    int reg = 0;

    while (reg != nFunc) {
        //le o arquivo e coloca no vetor
        TFunc *v[M];
        int i = 0;
        while (!feof(arq)) {
            fseek(arq, (reg) * tamanho_registro(), SEEK_SET);
            v[i] = le_funcionario(arq);
       //     imprime_funcionario(v[i]);
            i++;
            reg++;

            if(i>=M) break;
        }

        //ajusta tamanho M caso arquivo de entrada tenha terminado antes do vetor
        if (i != M) {
            M = i;
        }

        //faz ordenacao
        for (int j = 1; j < M; j++) {
            TFunc *f = v[j];
            i = j - 1;
            while ((i >= 0) && (v[i]->cod > f->cod)) {
                v[i + 1] = v[i];
                i = i - 1;
            }
            v[i + 1] = f;
        }

        //cria arquivo de particao e faz gravacao
        char *nome_particao = nome_arquivos_saida->nome;
        nome_arquivos_saida = nome_arquivos_saida->prox;
        printf("\n%s\n", nome_particao);
        FILE *p;
        if ((p = fopen(nome_particao, "wb+")) == NULL) {
            printf("Erro criar arquivo de saida\n");
        } else {
            for (int i = 0; i < M; i++) {
                fseek(p, (i) * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[i], p);
                imprime_funcionario(v[i]);
            }
            fclose(p);
        }
        for(int jj = 0; jj<M; jj++)
            free(v[jj]);
    }
}

void selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int *qtd_particoes){
    rewind(arq); //posiciona cursor no inicio do arquivo

    int reg = 0, count = 0, stop = 0, i = 0, aberto = 0, inicial = 0, pos = 0, r = 0;

    TFunc *salva_gravado, *menor_memoria, **v;

    v = calloc(sizeof(**v), M);
    salva_gravado = calloc(sizeof(*salva_gravado), 1);
    menor_memoria = NULL;

    FILE *reservar = fopen("reservatorio.dat", "wb+");
    FILE *p;

    if (reservar == NULL || v == NULL || salva_gravado == NULL){
        printf("Não foi possível abrir o arquivo e/ou alocar memória");
    }
    else{

        while (reg != nFunc) {
            //Lê os M primeiros registros do arquivo e coloca no vetor
             while (stop == 0) {
                fseek(arq, reg* tamanho_registro(), SEEK_SET);
                v[i] = le_funcionario(arq);
                i++;
                if(i>=M) stop = 1;
                else reg++;
            }

            //Ajusta tamanho M caso arquivo de entrada tenha terminado antes do vetor
            if (i != M) {
                M = i;
            }

            menor_memoria = encontra_menor(v, M, &pos);

            if (menor_memoria->cod > salva_gravado->cod || inicial == 0){
                inicial = 1;
                //Abrindo as partições
                char *nome_particao = nome_arquivos_saida->nome;

                if (aberto == 0){
                    printf("\n%s\n", nome_arquivos_saida->nome);
                    nome_arquivos_saida = nome_arquivos_saida->prox;
                    p = fopen(nome_particao, "wb+");
                    if (p == NULL) printf("Erro ao abrir o arquivo.\n");
                    aberto = 1;
                    *qtd_particoes+=1; //Para usar o algoritmo de intercalação básico
                    r = 0; //Troca de partições

                }
                //Salvando
                fseek(p, r*tamanho_registro(), SEEK_SET);
                salva_funcionario(menor_memoria, p);
                imprime_funcionario(menor_memoria);
                *salva_gravado = *menor_memoria;
                reg++;
                r++;
            }
            else{
                //Caso tenha espaço no reservatório e o registro na memória é menor que o último registro salvo
                if (count < M){
                    count = reservatorio(menor_memoria, count, reservar, M); //Foi para o reservatório
                    reg++;
                }
                else{
                    //Reservatorio cheio
                    count = 0;
                    aberto = 0;
                    ordena(v, M); //ordena os remanescentes

                    //Escreve na partição o que está na memória
                    for (int j = 0; j < M; j++){
                        fseek(p, r*tamanho_registro(), SEEK_SET);
                        salva_funcionario(v[j], p);
                        imprime_funcionario(v[j]);
                        r++;
                        free(v[j]);
                    }

                    //Colocar o reservatório na memória
                    for (int j = 0; j < M; j++){
                        fseek(reservar, j*tamanho_registro(), SEEK_SET);
                        v[j] = le_funcionario(reservar);
                    }

                    fclose(reservar);
                    reservar = fopen("reservatorio.dat", "wb+"); //Limpando reservatório
                    inicial = 0;
                    fclose(p);
                }
            }
            //Lê o próximo registro do arquivo
            if (inicial != 0 && reg < 20){
                fseek(arq, reg*tamanho_registro(), SEEK_SET);
                free(v[pos]);
                v[pos] = le_funcionario(arq);
            }
        }

    ordena(v, M);
    //Grava na partição o que sobrou na memória
    for (int j = 0; j < M; j++){
        if (v[j]->cod != salva_gravado->cod && v[j]->cod != menor_memoria->cod){
            fseek(p, r*tamanho_registro(), SEEK_SET);
            salva_funcionario(v[j], p);
            imprime_funcionario(v[j]);
            r++;
        }
    }

    int c = 0;
    //Tras o restante do reservatório para a memória
    for (int j = 0; fgetc(reservar) != EOF; j++){
        fseek(reservar, j*tamanho_registro(), SEEK_SET);
        v[j] = le_funcionario(reservar);
        c++;
    }

    ordena(v, M);
    //Grava o que estava no reservatório
    for (int j = 0; j < c; j++){
        fseek(p, r*tamanho_registro(), SEEK_SET);
        salva_funcionario(v[j], p);
        imprime_funcionario(v[j]);
        r++;
    }

    for (int i = 0; i < M; i++) free(v[i]);
    free(v);
    free(salva_gravado);
    fclose(p);
    fclose(reservar);
    }
}

TFunc *encontra_menor(TFunc **memoria, int M, int *posicao){
    TFunc *menor = memoria[0];
    *posicao = 0;
    for (int i=0; i<M; i++){
        if (memoria[i]->cod < menor->cod){
            menor = memoria[i];
            *posicao = i;
        }
    }

    return menor;
}

void ordena (TFunc **v, int M){
    //Faz ordenacao dos que já estão na memória para a partição
    int i = 0;
    for (int j = 1; j < M; j++) {
        TFunc *f = v[j];
        i = j - 1;
        while ((i >= 0) && (v[i]->cod > f->cod)) {
            v[i + 1] = v[i];
            i = i - 1;
        }
        v[i + 1] = f;
    }
}

int reservatorio (TFunc *func, int contador, FILE *reservatorio, int M){
    //Salva o registro no reservatório se não estiver cheio
    if (contador < M){
        fseek(reservatorio, (contador)*tamanho_registro(), SEEK_SET);
        salva_funcionario(func, reservatorio);
        contador++;
    }

    return contador;
}

//Para ordenar as partições e serem intercalados depois
void organiza_p (Lista *nome_arq, int qtd, int nFunc){
    char *nomes;
    FILE *pt;

    TFunc **v = calloc(sizeof(**v), nFunc);
    int c, t;

    //Traz da partição para memória, ordena, e salva de volta na partição
    for (int i = 0; i < qtd; i++){
        nomes = nome_arq->nome;
        pt = fopen (nomes, "rb+");

        c = 0;
        for (int j = 0; fgetc(pt) != EOF; j++){
            fseek (pt, j*tamanho_registro(), SEEK_SET);
            v[j] = le_funcionario(pt);
            c++;
        }

        ordena(v, c);
        t = 0;
        fclose(pt);
        pt = fopen(nomes, "wb+");

        while (t < c){
            fseek(pt, t*tamanho_registro(), SEEK_SET);
            salva_funcionario(v[t], pt);
            t++;
        }
        fclose(pt);
        nome_arq = nome_arq->prox;

        for (int k = 0; k < c; k++) free(v[k]);
    }
    free(v);
}

void exibe (Lista *nome_arq, int qtd, int nFunc){
    FILE *pt;
    char *nomes;
    TFunc **v = calloc(sizeof(**v), nFunc);

    printf("\n\nPARTIÇÕES ORDENADAS");

    for (int i = 0; i < qtd; i++){
        nomes = nome_arq->nome;
        pt = fopen (nomes, "rb+");
        printf("\n%s\n", nome_arq->nome);

        for (int j = 0; fgetc(pt) != EOF; j++){
            fseek (pt, j*tamanho_registro(), SEEK_SET);
            v[j] = le_funcionario(pt);
            imprime_funcionario(v[j]);
            free(v[j]);
        }
        fclose(pt);
        nome_arq = nome_arq->prox;
    }

    free(v);
}
