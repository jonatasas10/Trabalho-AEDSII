#include "intercalacao.h"
#include "funcionarios.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {

    int fim = 0; //variavel que controla fim do procedimento
    FILE *out; //declara ponteiro para arquivo

    //abre arquivo de saida para escrita
    if ((out = fopen(nome_arquivo_saida, "wb")) == NULL) {
        printf("Erro ao abrir arquivo de sa?da\n");
    } else {
        //cria vetor de particoes
        TVet v[num_p];

        //abre arquivos das particoes, colocando variavel de arquivo no campo f do vetor
        //e primeiro funcionario do arquivo no campo func do vetor
        for (int i=0; i < num_p; i++) {
            v[i].f = fopen(nome_particoes->nome, "rb");
            v[i].aux_p = 0;
            if (v[i].f != NULL) {
                fseek(v[i].f, v[i].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[i].f);
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posi??o do vetor
                    v[i].func = funcionario(INT_MAX, "","","",0);
                }
                else {
                    //conseguiu ler funcionario, coloca na posi??o atual do vetor
                    v[i].func = f;
                }
            }
            else {
                fim = 1;
            }
            nome_particoes = nome_particoes->prox;
        }

        int aux = 0;
        while (!(fim)) { //conseguiu abrir todos os arquivos
            int menor = INT_MAX;
            int pos_menor;
            //encontra o funcionario com menor chave no vetor
            for(int i = 0; i < num_p; i++){
                if(v[i].func->cod < menor){
                    menor = v[i].func->cod;
                    pos_menor = i;
                }
            }
            if (menor == INT_MAX) {
                fim = 1; //terminou processamento
            }
            else {
                //salva funcionario no arquivo de saída
                fseek(out, aux * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[pos_menor].func, out);
                //atualiza posição pos_menor do vetor com pr?ximo funcionario do arquivo
                v[pos_menor].aux_p++;
                fseek(v[pos_menor].f, v[pos_menor].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[pos_menor].f);
                aux++;
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posiçao do vetor
                    v[pos_menor].func = funcionario(INT_MAX, "", "", "",0.0);
                }
                else {
                    v[pos_menor].func = f;
                }

            }
        }

        //fecha arquivos das partiÇões de entrada
        for(int i = 0; i < num_p; i++){
            fclose(v[i].f);
        //    free(v[i].func);
        }
        //fecha arquivo de saída
        fclose(out);
    }
}

void intercalacao_arvore_de_vencedores(int qtd_p, Lista *nomes_p){

    int fim = 0, k = 0, tam, aloca = qtd_p;

    if (qtd_p%2 == 0){
        tam = 2*qtd_p - 1;
        aloca--;
    }
    else {
        tam = 2*qtd_p;
    }
    int aux = tam-1;

    TVet *list = calloc(sizeof(*list), qtd_p);
    TFunc *salva_menor = calloc(sizeof(*salva_menor), 1);
    TFunc **v = calloc(sizeof(**v), tam);
    for (int i = 0; i < aloca; i++) v[i] = calloc(sizeof(TFunc),1);

    FILE *saida = fopen("arquivo_intercalado.dat", "wb+");

    if (saida == NULL || list == NULL || salva_menor == NULL || v == NULL){
        printf("Não foi possível abrir o arquivo e/ou alocar memória.");
    }
    else{
        //Abre as partições e carrega os primeiros registros para a memória
        for (int i = 0; i < qtd_p; i++){
            list[i].aux_p = 0;
            char *nome = nomes_p->nome;
            list[i].f = fopen (nome, "rb+");
            fseek(list[i].f, 0*tamanho_registro(), SEEK_SET);

            if (aux+1>= qtd_p){
                v[aux] = le_funcionario(list[i].f);
                aux--;
            }
            list[i].fim_p = 0;
            nomes_p = nomes_p->prox;
        }

        vencedor(v, tam, saida, &k); //Primeiro vencedor

        while (fim < qtd_p){
            aux = tam-1;

            for (int i = 0; i < qtd_p; i++){
                //Se fim da partição
                if (fgetc(list[i].f) == EOF && list[i].fim_p == 0 && salva_menor->cod == v[aux]->cod){
                    fim++;
                    list[i].fim_p = 1;
                    i--;
                    if (fim == qtd_p) break;

                    //Substituir todas as ocorrências
                    for (int j = 0; j < tam; j++){
                        if (v[j]->cod == salva_menor->cod) v[j]->cod = 15000;
                    }
                    //Temos que achar o novo vencedor
                    vencedor(v, tam, saida, &k);
                }
                else{
                    *salva_menor = *v[0];

                    if (salva_menor->cod == v[aux]->cod && list[i].fim_p == 0){
                        list[i].aux_p += 1;

                        if (fgetc(list[i].f) != EOF){
                            fseek (list[i].f, (list[i].aux_p)*tamanho_registro(), SEEK_SET);
                            free(v[aux]);
                            v[aux] = le_funcionario(list[i].f);
                            vencedor(v, tam, saida, &k);
                        }
                    }
                    aux--;
                }
            }
        }

        for (int i = 0; i < qtd_p; i++) fclose(list[i].f);

        for (int i = 0; i < tam; i++) free(v[i]);

        free(v);
        free(salva_menor);
        free(list);
        fclose(saida);
    }

}

void vencedor (TFunc **func, int n, FILE *saida, int *c){
    int pos;

    for (int i = n-1; i>0; i--){

        if (i%2 != 0 && i == n-1){ //Um filho só = posição na memória = ímpar
            pos = (i-1)/2;
            *func[pos] = *func[i];
        }
        else{
            if (i%2 == 0 && func[i]->cod < func[i-1]->cod){ //Caso filho da direita for menor que esquerda
                pos = (i-2)/2;
                *func[pos] = *func[i];
            }
            else if (i%2 == 0 && func[i-1]->cod < func[i]->cod){ //O contrário
                pos = (i-2)/2;
                *func[pos] = *func[i-1];
            }
        }
    }
    fseek(saida, (*c)*tamanho_registro(), SEEK_SET);
    salva_funcionario(func[0], saida);
    *c +=1;
}
