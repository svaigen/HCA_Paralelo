#include "parallel_structures.h"

void buffer_tarefas_inicializa(int tamanho, struct_buffer_tarefas* b) {
    int i;
    for (i = 0; i < tamanho; i++) {
        b->buffer[i] = init_solution();
    }
    b->pos_remocao = 0;
    b->pos_insercao = 0;
    b->fim_fisico = tamanho;
}

void buffer_tarefas_add(struct_buffer_tarefas* b, int n) {
    //printf("pos ins antes -> %d\n",l->pos_insercao);
    b->buffer[b->pos_insercao] = n;
    b->pos_insercao = (b->pos_insercao + 1) % b->fim_fisico;
    //printf("pos ins depois -> %d\n",l->pos_insercao);
}

gcp_solution_t buffer_tarefas_remove(struct_buffer_tarefas* b) {
    gcp_solution_t ret;
    ret = b->buffer[b->pos_remocao];
    b->pos_remocao = (b->pos_remocao + 1) % b->fim_fisico;
    return ret;
}

void buffer_individuos_inicializa(int tamanho, struct_buffer_individuos* b) {
    int i;
    for (i = 0; i < tamanho; i++) {
        b->buffer[i] = init_solution();
    }
    b->pos_insercao = 0;
    b->fim_logico = tamanho;
}

void buffer_individuos_add(struct_buffer_individuos* b, int n) {
    if (buffer_individuos_is_cheio(b)) {
        perror("ERRO! O Buffer de individuos está cheio. O programa sera finalizado.\n");
        exit(-1);
    } else {
        b->buffer[b->pos_insercao] = n;
        b->pos_insercao++;
    }
}

void buffer_individuos_esvazia(struct_buffer_individuos* b) {
    b->pos_insercao = 0;
}

int buffer_individuos_is_cheio(struct_buffer_individuos* b) {
    if (b->fim_logico == b->pos_insercao) {
        return 1;
    }
    return 0;
}