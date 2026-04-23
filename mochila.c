#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_ITEMS 64
#define INITIAL_DEPTH 3   // profundidad inicial para crear tareas/subárboles

typedef struct {
    int id_original;
    int weight;
    int profit;
    double ratio;
} Item;

typedef struct {
    int level;                  // próximo ítem a decidir
    int weight;
    int profit;
    double bound;
    int taken[MAX_ITEMS];       // 1 si se toma, 0 si no
} Node;

typedef struct {
    Node nodes[1 << (INITIAL_DEPTH + 2)];
    int count;
} Frontier;

int N;
int CAPACITY;
Item items[MAX_ITEMS];

int best_profit = 0;
int best_taken[MAX_ITEMS] = {0};

/* PROTOTIPOS */
int cmp_items(const void *a, const void *b);
double compute_bound(Node *u);
void try_update_best(Node *u);
void dfs_branch_bound(Node start);
void generate_frontier(Frontier *frontier);
void print_solution();

/* DEFINICIÓN */
int cmp_items(const void *a, const void *b) {
    const Item *x = (const Item *)a;
    const Item *y = (const Item *)b;

    if (y->ratio > x->ratio) return 1;
    if (y->ratio < x->ratio) return -1;
    return 0;
}

/*----------------------------------------------------------
  Cota superior tipo mochila fraccional
----------------------------------------------------------*/
double compute_bound(Node *u) {
    if (u->weight >= CAPACITY) return 0.0;

    double bound = u->profit;
    int total_weight = u->weight;
    int j = u->level;

    while (j < N && total_weight + items[j].weight <= CAPACITY) {
        total_weight += items[j].weight;
        bound += items[j].profit;
        j++;
    }

    if (j < N) {
        bound += (CAPACITY - total_weight) * items[j].ratio;
    }

    return bound;
}

/*----------------------------------------------------------
  Actualiza la mejor solución global con sincronización
----------------------------------------------------------*/
void try_update_best(Node *u) {
    if (u->profit > best_profit) {
        #pragma omp critical
        {
            if (u->profit > best_profit) {
                best_profit = u->profit;
                memcpy(best_taken, u->taken, sizeof(int) * N);
            }
        }
    }
}

/*----------------------------------------------------------
  DFS Branch & Bound secuencial sobre un subárbol.
  Esta función será ejecutada por distintos threads sobre
  distintos nodos raíz del frontier.
----------------------------------------------------------*/
void dfs_branch_bound(Node start) {
    Node stack[1024];
    int top = 0;
    stack[top++] = start;

    while (top > 0) {
        Node u = stack[--top];

        // poda por cota
        if (u.bound <= best_profit) {
            continue;
        }

        // si ya decidimos todos los ítems
        if (u.level >= N) {
            try_update_best(&u);
            continue;
        }

        int idx = u.level;

        // ===== Rama 1: incluir item idx =====
        Node with = u;
        with.level = idx + 1;
        with.weight += items[idx].weight;
        with.profit += items[idx].profit;
        with.taken[idx] = 1;

        if (with.weight <= CAPACITY) {
            if (with.profit > best_profit) {
                try_update_best(&with);
            }
            with.bound = compute_bound(&with);
            if (with.bound > best_profit) {
                stack[top++] = with;
            }
        }

        // ===== Rama 2: excluir item idx =====
        Node without = u;
        without.level = idx + 1;
        without.taken[idx] = 0;
        without.bound = compute_bound(&without);

        if (without.bound > best_profit) {
            stack[top++] = without;
        }
    }
}

/*----------------------------------------------------------
  Genera subárboles iniciales hasta INITIAL_DEPTH
  para repartir trabajo entre threads.
----------------------------------------------------------*/
void generate_frontier(Frontier *frontier) {
    frontier->count = 0;

    Node root;
    root.level = 0;
    root.weight = 0;
    root.profit = 0;
    memset(root.taken, 0, sizeof(root.taken));
    root.bound = compute_bound(&root);

    Node queue[1024];
    int qh = 0, qt = 0;
    queue[qt++] = root;

    while (qh < qt) {
        Node u = queue[qh++];

        if (u.level >= INITIAL_DEPTH || u.level >= N) {
            frontier->nodes[frontier->count++] = u;
            continue;
        }

        int idx = u.level;

        // Rama incluir
        Node with = u;
        with.level = idx + 1;
        with.weight += items[idx].weight;
        with.profit += items[idx].profit;
        with.taken[idx] = 1;

        if (with.weight <= CAPACITY) {
            with.bound = compute_bound(&with);
            if (with.bound > best_profit) {
                queue[qt++] = with;
            }
        }

        // Rama excluir
        Node without = u;
        without.level = idx + 1;
        without.taken[idx] = 0;
        without.bound = compute_bound(&without);

        if (without.bound > best_profit) {
            queue[qt++] = without;
        }
    }
}

/*----------------------------------------------------------
  Muestra la solución final
----------------------------------------------------------*/
void print_solution() {
    printf("\nMejor beneficio encontrado = %d\n", best_profit);

    int total_weight = 0;
    printf("Items seleccionados (en orden densidad):\n");
    for (int i = 0; i < N; i++) {
        if (best_taken[i]) {
            printf("  item_ordenado=%d (id_original=%d, peso=%d, valor=%d)\n",
                   i, items[i].id_original, items[i].weight, items[i].profit);
            total_weight += items[i].weight;
        }
    }
    printf("Peso total = %d / %d\n", total_weight, CAPACITY);
}

int main() {
    // Ejemplo clásico
    N = 10;
    CAPACITY = 16;

    int weights[] = {2, 5, 10, 5, 7, 3, 1, 4, 6, 8};
    int profits[] = {40, 30, 50, 10, 35, 40, 30, 50, 45, 60};

    for (int i = 0; i < N; i++) {
        items[i].id_original = i;
        items[i].weight = weights[i];
        items[i].profit = profits[i];
        items[i].ratio = (double)profits[i] / weights[i];
    }

    // ordenar por ratio descendente
    qsort(items, N, sizeof(Item), cmp_items);

    printf("Items ordenados por valor/peso:\n");
    for (int i = 0; i < N; i++) {
        printf("  [%d] orig=%d peso=%d valor=%d ratio=%.2f\n",
               i, items[i].id_original, items[i].weight,
               items[i].profit, items[i].ratio);
    }

    Frontier frontier;
    generate_frontier(&frontier);

    printf("\nSubarboles iniciales generados: %d\n", frontier.count);

    double t0 = omp_get_wtime();

    // ---------------------------------------------------
    // PARALELIZACIÓN EXPLÍCITA EN THREADS
    // Cada iteración del for toma un subárbol distinto
    // ---------------------------------------------------
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < frontier.count; i++) {
        int tid = omp_get_thread_num();

        printf("Thread %d explora subarbol %d (level=%d, profit=%d, weight=%d, bound=%.2f)\n",
               tid, i, frontier.nodes[i].level, frontier.nodes[i].profit,
               frontier.nodes[i].weight, frontier.nodes[i].bound);

        dfs_branch_bound(frontier.nodes[i]);
    }

    double t1 = omp_get_wtime();

    print_solution();

    printf("\nTiempo total paralelo: %.6f segundos\n", t1 - t0);
    printf("Threads usados: %d\n", omp_get_max_threads());

    return 0;
}