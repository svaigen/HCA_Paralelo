/* 
 * File:   parallel_structures.h
 * Author: svaigen
 *
 * Created on May 4, 2015, 3:38 PM
 */

#ifndef PARALLEL_STRUCTURES_H
#define	PARALLEL_STRUCTURES_H

#include <semaphore.h>
#include <pthread.h>
#include "color.h"


#define MAX_TAM_BUFFER_TAREFAS 1024
#define MAX_TAM_BUFFER_INDIVIDUOS 1024

struct struct_buffer_tarefas{
    gcp_solution_t* buffer[MAX_TAM_BUFFER_TAREFAS];
    int buffer_parents[MAX_TAM_BUFFER_TAREFAS * 2];
    int pos_remocao;
    int pos_insercao;
    int fim_fisico;
};

struct struct_buffer_individuos {
    gcp_solution_t* buffer[MAX_TAM_BUFFER_INDIVIDUOS];
    int buffer_parents[MAX_TAM_BUFFER_INDIVIDUOS * 2];
    int fim_logico;
    int pos_insercao;
};

typedef struct struct_buffer_tarefas struct_buffer_tarefas;
typedef struct struct_buffer_individuos struct_buffer_individuos;

int n_threads;
int tam_buffer_tarefas;
int tam_buffer_novos_individuos;
struct_buffer_individuos buffer_novos_individuos;
struct_buffer_tarefas buffer_tarefas;

sem_t sem_mutex_tarefas;
sem_t sem_is_cheio_tarefas;
sem_t sem_is_vazio_tarefas;
sem_t sem_mutex_individuos;
sem_t sem_preenche_individuos;
sem_t sem_atualiza_populacao;
sem_t sem_mutex_populacao;


void buffer_tarefas_inicializa(int tamanho, struct_buffer_tarefas* b);
void buffer_tarefas_add(struct_buffer_tarefas* b, gcp_solution_t *s, int parent1, int parent2);
gcp_solution_t* buffer_tarefas_remove(struct_buffer_tarefas* b);
void buffer_tarefas_get_parents(struct_buffer_tarefas *b, int pos, int *parent1, int *parent2);
void buffer_individuos_inicializa(int tamanho, struct_buffer_individuos* b);
void buffer_individuos_add(struct_buffer_individuos* b, gcp_solution_t *s, int parent1, int parent2);
void buffer_individuos_esvazia(struct_buffer_individuos* b);
void buffer_individuos_get_parents(struct_buffer_individuos *b, int pos, int *parent1, int *parent2);
int buffer_individuos_is_cheio(struct_buffer_individuos* b);
void buffer_individuos_seleciona_melhor(struct_buffer_individuos* b, gcp_solution_t *solution, int *parent1, int *parent2);
#endif	/* PARALLEL_STRUCTURES_H */