#include <stdlib.h>

#include "parallel_structures.h"
#include "util.h"

void buffer_tarefas_inicializa(int tamanho, struct_buffer_tarefas* b) {
    int i;
    for (i = 0; i < tamanho; i++) {
        b->buffer[i] = init_solution();
    }
    b->pos_remocao = 0;
    b->pos_insercao = 0;
    b->fim_fisico = tamanho;
}

void buffer_tarefas_add(struct_buffer_tarefas* b, gcp_solution_t *n, int parent1, int parent2) {
    //printf("pos ins antes -> %d\n",l->pos_insercao);
    cpy_solution(n, b->buffer[b->pos_insercao]);
    //b->buffer[b->pos_insercao] = n;
    b->buffer_parents[b->pos_insercao * 2] = parent1;
    b->buffer_parents[b->pos_insercao * 2 + 1] = parent2;
    b->pos_insercao = (b->pos_insercao + 1) % b->fim_fisico;
    //printf("pos ins depois -> %d\n",l->pos_insercao);
}

gcp_solution_t* buffer_tarefas_remove(struct_buffer_tarefas* b) {
    gcp_solution_t* ret = init_solution();
    cpy_solution(b->buffer[b->pos_remocao], ret);
    //ret = b->buffer[b->pos_remocao];
    b->pos_remocao = (b->pos_remocao + 1) % b->fim_fisico;
    return ret;
}

void buffer_tarefas_get_parents(struct_buffer_tarefas *b, int pos, int *parent1, int *parent2) {
    *parent1 = b->buffer_parents[pos * 2];
    *parent2 = b->buffer_parents[pos * 2 + 1];
}

void buffer_individuos_inicializa(int tamanho, struct_buffer_individuos* b) {
    int i;
    for (i = 0; i < tamanho; i++) {
        b->buffer[i] = init_solution();
    }
    b->pos_insercao = 0;
    b->fim_logico = tamanho;
}

void buffer_individuos_add(struct_buffer_individuos* b, gcp_solution_t *s, int parent1, int parent2) {
    if (buffer_individuos_is_cheio(b)) {
        perror("ERRO! O Buffer de individuos está cheio. O programa sera finalizado.\n");
        exit(EXIT_FAILURE);
    } else {
        cpy_solution(s, b->buffer[b->pos_insercao]);
        //b->buffer[b->pos_insercao] = s;
        b->buffer_parents[b->pos_insercao * 2] = parent1;
        b->buffer_parents[b->pos_insercao * 2 + 1] = parent2;
        b->pos_insercao++;
    }
}

void buffer_individuos_esvazia(struct_buffer_individuos* b) {
    int tamanho = b->fim_logico;
    int i;
    for (i = 0; i < tamanho; i++) {
        free(b->buffer[i]);
    }
    buffer_individuos_inicializa(tamanho, b);
    //b->pos_insercao = 0;
}

void buffer_individuos_get_parents(struct_buffer_individuos *b, int pos, int *parent1, int *parent2) {
    *parent1 = b->buffer_parents[pos * 2];
    *parent2 = b->buffer_parents[pos * 2 + 1];
}

int buffer_individuos_is_cheio(struct_buffer_individuos* b) {
    if (b->fim_logico == b->pos_insercao) {
        return 1;
    }
    return 0;
}

void buffer_individuos_seleciona_melhor(struct_buffer_individuos* b, gcp_solution_t *offspring, int *parent1, int *parent2) {
    int best = b->buffer[0]->nof_confl_edges;
    int pos = 0;
    int i;
    for (i = 1; i < b->fim_logico; i++) {
        if (b->buffer[i]->nof_confl_edges < best) {
            best = b->buffer[i]->nof_confl_edges;
            pos = i;
        }

    }
    cpy_solution(b->buffer[pos], offspring);
    *parent1 = b->buffer_parents[pos * 2];
    *parent2 = b->buffer_parents[pos * 2 + 1];
}