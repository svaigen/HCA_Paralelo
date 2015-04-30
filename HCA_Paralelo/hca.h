/**********************************************************
 * Created: Ter 09 Ago 2011 21:23:15 BRT
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
 * Trying to make it better
 * 
 **********************************************************/
#ifndef __HCA_H
#define __HCA_H

#define HCA_DEFAULT_POP 10
#define HCA_DEFAULT_LS 2000
#define HCA_DEFAULT_CYC 5000
#define HCA_DEFAULT_DIVERSITY 20

struct hca_t {
  int sizeof_population;
  int cyc_local_search;
  int seed;
  int nof_cross;
  int diversity;
};

typedef struct hca_t hca_t;

hca_t *hca_info;

void hca_printbanner(void);
void hca_parseargs(void);
void hca_initialization(void);
void hca_show_solution(void);
gcp_solution_t* hca(void);
 
#endif /* __HCA_H */
