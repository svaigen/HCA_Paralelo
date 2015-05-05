/***********************************************************
 * Created: Ter 09 Ago 2011 21:05:59 BRT
 *
 * Author: Carla N. Lintzmayer, carla0negri@gmail.com
 *
 ***********************************************************
 *
 * HCA [1999, , Galinier and Hao]
 *
 * Hybrid Coloring Algorithm.
 * Utilizing GPX crossover.
 *
 ***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "color.h"
#include "util.h"
#include "tabucol.h"
#include "hca.h"
#include "parallel_structures.h"

static gcp_solution_t **population;
static gcp_solution_t *best_solution;
int cross, converg, cycle;

extern int tam_buffer_tarefas;
extern int tam_buffer_novos_individuos;
extern struct_buffer_individuos buffer_novos_individuos;
extern struct_buffer_tarefas buffer_tarefas;
extern int n_threads;

extern sem_t sem_mutex_tarefas;
extern sem_t sem_is_cheio_tarefas;
extern sem_t sem_is_vazio_tarefas;
extern sem_t sem_mutex_individuos;
extern sem_t sem_preenche_individuos;
extern sem_t sem_atualiza_populacao;
extern sem_t sem_mutex_populacao;

void hca_printbanner(void) {
    fprintf(problem->fileout, "HCA\n");
    fprintf(problem->fileout, "-------------------------------------------------\n");
    fprintf(problem->fileout, "Using Parameters:\n");
    fprintf(problem->fileout, "  Population size..: %d\n", hca_info->sizeof_population);
    fprintf(problem->fileout, "  Max LS iter......: %d\n", hca_info->cyc_local_search);
    fprintf(problem->fileout, "  Max colors.......: %i\n", problem->max_colors);
    fprintf(problem->fileout, "  Seed.............: %i\n", hca_info->seed);
}

void hca_initialization(void) {
    if (!(get_flag(problem->flags, FLAG_COLOR)))
        problem->max_colors = problem->nof_vertices;

    if (!(get_flag(problem->flags, FLAG_SEED)))
        hca_info->seed = create_seed();

    srand48(hca_info->seed);
}

void hca_show_solution(void) {
    fprintf(problem->fileout, "Nof. crossovers realized: %d\n", hca_info->nof_cross);
    fprintf(problem->fileout, "Diversity in the final population: %d\n", hca_info->diversity);
}

static void test_solution(gcp_solution_t* sol) {
    int i, j, v;
    for (i = 0; i < problem->max_colors; i++) {
        for (j = 1; j <= sol->class_color[i][0]; j++) {
            v = sol->class_color[i][j];
            if (i != sol->color_of[v]) {
                printf(" ERROR!! %d está na classe %d, mas cor de %d = %d \n", v + 1, i, v + 1, sol->color_of[v]);
            }
        }
    }
}

static gcp_solution_t* create_indiv(void) {

    gcp_solution_t *solution = init_solution();
    int i, j, c, v, color, nc, v_max_degree;
    int possible_color[problem->nof_vertices];
    //    int confl_vertices[problem->nof_vertices];
    int neighbors_by_color[problem->nof_vertices][problem->max_colors + 1];

    /* Initializing auxiliary arrays and choosing a vertex with a maximal degree
     * to be the first one */
    v_max_degree = 0;
    for (i = 0; i < problem->nof_vertices; i++) {
        possible_color[i] = 0;
        solution->color_of[i] = -1;
        //		confl_vertices[i] = 0;
        for (j = 0; j < problem->max_colors; j++) {
            neighbors_by_color[i][j] = 0;
            solution->class_color[j][i] = -1;
        }
        neighbors_by_color[i][problem->max_colors] = 0;
        if (problem->degree[i] > problem->degree[v_max_degree]) {
            v_max_degree = i;
        }
    }
    for (j = 0; j < problem->max_colors; j++) {
        solution->class_color[j][problem->nof_vertices] = -1;
        solution->class_color[j][0] = 0;
    }

    v_max_degree = 0;
    /* Color the chosen vertex with the first color (0) */
    color = 0;
    solution->color_of[v_max_degree] = color;
    solution->class_color[color][0]++;
    solution->class_color[color][solution->class_color[color][0]] = v_max_degree;
    v = v_max_degree; /* the current vertex, last one that was colored */
    nc = 1; /* number of colored vertices */

    while (nc < problem->nof_vertices) {

        /* Update degree of saturation and possible colors; choose vertex with
         * maximal saturation degree */
        for (i = 0; i < problem->nof_vertices; i++) {
            if (problem->adj_matrix[v][i]) {
                /* update degree of saturation: */
                if (neighbors_by_color[i][color] == 0) {
                    neighbors_by_color[i][problem->max_colors]++;
                }
                /* now <i> has a neighbor colored with <color> */
                neighbors_by_color[i][color]++;

                if (solution->color_of[i] == -1) {
                    /* if <i> is not colored yet and <i> is neighbor of <v>,
                     * update possible color for <i>: among all the possible 
                     * colors for a neighbor of <i>, chose the least one */
                    int changed = FALSE;
                    for (c = problem->max_colors; c >= 0; c--) {
                        if (neighbors_by_color[i][c] == 0) {
                            possible_color[i] = c;
                            changed = TRUE;
                        }
                    }
                    if (!changed) possible_color[i] = problem->max_colors;
                }
            }
        }

        v_max_degree = -1;
        for (i = 0; i < problem->nof_vertices; i++) {
            /* choose vertex with a maximal saturation degree: */
            if (solution->color_of[i] == -1) {
                if (v_max_degree == -1) v_max_degree = i;
                else if (neighbors_by_color[i][problem->max_colors] >
                        neighbors_by_color[v_max_degree][problem->max_colors]) {
                    v_max_degree = i;
                }
            }
        }

        v = v_max_degree;
        color = possible_color[v];

        /* if no viable color is found for <v>, chose a random one.
         * this means that a conflict is being generated. */
        if (color == problem->max_colors) {
            color = (int) RANDOM(problem->max_colors);
        }

        solution->color_of[v] = color;
        solution->class_color[color][0]++;
        solution->class_color[color][solution->class_color[color][0]] = v;
        nc++;

    }

    solution->spent_time = current_usertime_secs();
    solution->time_to_best = solution->spent_time;

    test_solution(solution);

    return solution;

}

static void create_population(void) {

    int i;
    population = malloc_(sizeof (gcp_solution_t*) * hca_info->sizeof_population);
    best_solution->nof_confl_edges = INT_MAX;

    for (i = 0; i < hca_info->sizeof_population; i++) {
        population[i] = create_indiv();
        population[i]->cycles_to_best = 0;
        population[i]->nof_colors = problem->max_colors;
        tabucol(population[i], hca_info->cyc_local_search, tabucol_info->tl_style);
        population[i]->time_to_best = current_usertime_secs();

        if (population[i]->nof_confl_edges < best_solution->nof_confl_edges) {
            cpy_solution(population[i], best_solution);
        }
    }

}

static void choose_parents(int *p1, int *p2) {
    (*p1) = (int) RANDOM(hca_info->sizeof_population);
    (*p2) = (int) RANDOM(hca_info->sizeof_population);
    while ((*p2) == (*p1)) {
        (*p2) = (int) RANDOM(hca_info->sizeof_population);
    }
}

static void crossover(int p1, int p2, gcp_solution_t *offspring) {
    int color, parent, max, i, j, c, v, otherparent, p;
    int class_colors[2][problem->max_colors][problem->nof_vertices + 1];

    offspring->nof_colors = problem->max_colors;
    for (i = 0; i < problem->nof_vertices; i++) {
        offspring->color_of[i] = -1;
        for (c = 0; c < problem->max_colors; c++) {
            class_colors[0][c][i] = population[p1]->class_color[c][i];
            class_colors[1][c][i] = population[p2]->class_color[c][i];
            offspring->class_color[c][i] = -1;
        }
    }
    for (c = 0; c < problem->max_colors; c++) {
        offspring->class_color[c][0] = 0;
        offspring->class_color[c][problem->nof_vertices] = -1;
        class_colors[0][c][problem->nof_vertices] =
                population[p1]->class_color[c][problem->nof_vertices];
        class_colors[1][c][problem->nof_vertices] =
                population[p2]->class_color[c][problem->nof_vertices];
    }

    for (color = 0; color < problem->max_colors; color++) {
        parent = (color % 2) ? 0 : 1; // 0 equals p1 and 1 equals p2
        otherparent = (parent == 0) ? 1 : 0;

        /* choose <max> such as C_max of <parent> is maximal */
        max = 0;
        for (i = 1; i < problem->max_colors; i++) {
            if (class_colors[parent][i][0] > class_colors[parent][max][0]) {
                max = i;
            }
        }

        /* C_color gets C_max from <parent> */
        for (i = 1; i <= class_colors[parent][max][0]; i++) {

            v = class_colors[parent][max][i];
            class_colors[parent][max][i] = -1;

            offspring->class_color[color][0]++;
            offspring->class_color[color][offspring->class_color[color][0]] = v;
            offspring->color_of[v] = color;

            /* Remove all vertices of <otherparent> */
            p = (otherparent == 0) ? p1 : p2;
            c = population[p]->color_of[v];
            for (j = 1; j <= class_colors[otherparent][c][0]; j++) {
                if (class_colors[otherparent][c][j] == v) {
                    class_colors[otherparent][c][j] =
                            class_colors[otherparent][c][class_colors[otherparent][c][0]];
                    class_colors[otherparent][c][class_colors[otherparent][c][0]] = -1;
                    class_colors[otherparent][c][0]--;
                    break;
                }
            }
        }

        /* Remove all vertices in C_color of <parent> */
        class_colors[parent][max][0] = 0;

    }

    /* Assign randomly the vertices of V - (C_1 U ... U C_k) */
    for (i = 0; i < problem->nof_vertices; i++) {
        if (offspring->color_of[i] == -1) {
            c = (int) RANDOM(problem->max_colors);
            offspring->color_of[i] = c;
            offspring->class_color[c][0]++;
            offspring->class_color[c][offspring->class_color[c][0]] = i;
        }
    }

    test_solution(population[p1]);
    test_solution(population[p2]);
    test_solution(offspring);

}

static int substitute_worst(int p1, int p2, gcp_solution_t* offspring) {/*{{{*/
    if (population[p1]->nof_confl_edges > population[p2]->nof_confl_edges) {
        cpy_solution(offspring, population[p1]);
        return p1;
    }

    cpy_solution(offspring, population[p2]);
    return p2;
}

static int distance(gcp_solution_t *ind1, gcp_solution_t *ind2) {/*{{{*/
    int i, dist = 0;

    for (i = 0; i < problem->nof_vertices; i++) {
        if (ind1->color_of[i] != ind2->color_of[i]) {
            dist++;
        }
    }

    return dist;
}

static int calculate_diversity(void) {
    int i, j;
    double dist, diversity = 0;

    for (i = 0; i < hca_info->sizeof_population; i++) {
        dist = 0;
        for (j = 0; j < hca_info->sizeof_population; j++) {
            if (i != j) {
                dist += distance(population[i], population[j]);
            }
        }
        dist = (double) dist / (hca_info->sizeof_population - 1);
        diversity += dist;
    }
    diversity = (double) diversity / hca_info->sizeof_population;

    return diversity;
}

static int hca_terminate_conditions(gcp_solution_t *solution, int diversity) {

    if (diversity < HCA_DEFAULT_DIVERSITY) {
        solution->stop_criterion = STOP_ALG;
        return TRUE;
    } else if (best_solution->nof_confl_edges == 0) {
        solution->stop_criterion = STOP_BEST;
        return TRUE;
    }
    return FALSE;

}

void *produz(void* id) {
    cross = 0;
    while (TRUE) {
        //printf("PRODUZ\n");
        int parent1, parent2;
        choose_parents(&parent1, &parent2);
        gcp_solution_t *offspring = init_solution();
        crossover(parent1, parent2, offspring);
        sem_wait(&sem_is_vazio_tarefas);
        sem_wait(&sem_mutex_tarefas);
        buffer_tarefas_add(&buffer_tarefas, offspring, parent1, parent2);
        sem_post(&sem_mutex_tarefas);
        sem_post(&sem_is_cheio_tarefas);
        cross++;
    }
}

void *consome(void* id) {
    while (TRUE) {
        //printf("CONSOME\n");
        gcp_solution_t *offspring = init_solution();
        int parent1, parent2;
        sem_wait(&sem_is_cheio_tarefas);
        sem_wait(&sem_mutex_tarefas);
        buffer_tarefas_get_parents(&buffer_tarefas, buffer_tarefas.pos_remocao, &parent1, &parent2);
        offspring = buffer_tarefas_remove(&buffer_tarefas);
        sem_post(&sem_mutex_tarefas);
        sem_post(&sem_is_vazio_tarefas);
        //printf("SAI DO CONSOME\n");
        tabucol(offspring, hca_info->cyc_local_search, tabucol_info->tl_style);
        sem_wait(&sem_preenche_individuos);
        sem_wait(&sem_mutex_individuos);
        buffer_individuos_add(&buffer_novos_individuos, offspring, parent1, parent2);
        if (buffer_individuos_is_cheio(&buffer_novos_individuos)) {
            sem_post(&sem_atualiza_populacao);
        }
        sem_post(&sem_mutex_individuos);
    }
}

void *atualiza_populacao(void* id) {
    int parent1, parent2;
    gcp_solution_t *offspring = init_solution();
    while (cycle < 5) {
        //printf("ATUALIZA\n");
        sem_wait(&sem_atualiza_populacao);
        buffer_individuos_seleciona_melhor(&buffer_novos_individuos, offspring, &parent1, &parent2);
        sem_wait(&sem_mutex_individuos);
        buffer_individuos_esvazia(&buffer_novos_individuos);
        sem_post(&sem_mutex_individuos);
        int i;
        for (i = 0; i < buffer_novos_individuos.fim_logico; i++) {
            sem_post(&sem_preenche_individuos);
            int *n = malloc(sizeof (int));
            sem_getvalue(&sem_preenche_individuos, n);
        }
        sem_wait(&sem_mutex_populacao);
        int sp = substitute_worst(parent1, parent2, offspring);
        sem_post(&sem_mutex_populacao);
        if (best_solution->nof_confl_edges > offspring->nof_confl_edges) {
            cpy_solution(offspring, best_solution);
            best_solution->time_to_best = current_usertime_secs();
            best_solution->cycles_to_best = cycle;
            converg = 0;
        }

        hca_info->diversity = calculate_diversity();

        if (get_flag(problem->flags, FLAG_VERBOSE)) {
            fprintf(problem->fileout, "HCA: cycle %d; best so far: %d; diversity: %d; parent substituted: %d\n",
                    cycle, best_solution->nof_confl_edges, hca_info->diversity, sp + 1);
        }
        printf("cycle: %d\n",cycle);
        cycle++;
        converg++;
        //printf("CHEGA NO FINAL DA ATT\n");
    }
}

gcp_solution_t* hca(void) {

    /*Inicializacao dos componentes de programacao paralela*/
    tam_buffer_tarefas = 2 * n_threads;
    tam_buffer_novos_individuos = n_threads;
    buffer_tarefas_inicializa(tam_buffer_tarefas, &buffer_tarefas);
    buffer_individuos_inicializa(tam_buffer_novos_individuos, &buffer_novos_individuos);
    sem_init(&sem_mutex_tarefas, 0, 1); //semaforo binario
    sem_init(&sem_is_cheio_tarefas, 0, 0); //semaforo de 0 a tam_buffer_tarefas
    sem_init(&sem_is_vazio_tarefas, 0, tam_buffer_tarefas); //semaforo de 0 a tam_buffer_tarefas
    sem_init(&sem_preenche_individuos, 0, tam_buffer_novos_individuos); //semaforo de 0 a tam_buffer_novos_individuos
    sem_init(&sem_atualiza_populacao, 0, 0); //semaforo binario
    sem_init(&sem_mutex_individuos, 0, 1); //semaforo binario
    sem_init(&sem_mutex_populacao, 0, 1); //semaforo binario
    pthread_t threads[n_threads + 2];

    hca_info->diversity = 2 * HCA_DEFAULT_DIVERSITY;

    best_solution = init_solution();
    best_solution->nof_confl_edges = INT_MAX;
    create_population();

    //while (!hca_terminate_conditions(best_solution, hca_info->diversity) &&
    //        !terminate_conditions(best_solution, cycle, converg)) {

    /*Thread 1 --> Produz*/
    int id = 1;
    pthread_create(&threads[0], NULL, produz, &id);

    /*Thread 2 --> Consumidor*/
    for (id = 1; id <= n_threads; id++) {
        int *n = malloc(sizeof (int));
        *n = id;
        pthread_create(&threads[id], NULL, consome, n);
    }

    /*Thread 3 --> Atualizar*/
    pthread_create(&threads[n_threads + 1], NULL, atualiza_populacao, &id);

    //}

    pthread_join(threads[n_threads + 1], NULL);
    int i;
    for (i = 0; i <= n_threads; i++) {
        pthread_cancel(threads[i]);
    }
    /*Retorno do codigo sequencial*/
    best_solution->spent_time = current_usertime_secs();
    best_solution->total_cycles = cycle;
    hca_info->nof_cross = cross;

    return best_solution;

}


