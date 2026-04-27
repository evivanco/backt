#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_ITEMS 64
#define INITIAL_DEPTH 3

typedef struct {
    int id_original;
    double weight;
    double volume;
    int value;
    double relative_use;
    double priority_ratio;
} Package;

typedef struct {
    int level;
    double weight;
    double volume;
    int value;
    double bound;
    int taken[MAX_ITEMS];
} Node;

typedef struct {
    Node nodes[1024];
    int count;
} Frontier;

int N;
double MAX_WEIGHT;
double MAX_VOLUME;

Package packages[MAX_ITEMS];

int best_value = 0;
int best_taken[MAX_ITEMS] = {0};

/* Prototipos */
int cmp_packages(const void *a, const void *b);
double compute_bound(Node *u);
void try_update_best(Node *u);
void dfs_branch_bound(Node start);
void generate_frontier(Frontier *frontier);
void print_solution();

/* Ordena de mayor a menor prioridad logística */
int cmp_packages(const void *a, const void *b) {
    const Package *x = (const Package *)a;
    const Package *y = (const Package *)b;

    if (y->priority_ratio > x->priority_ratio) return 1;
    if (y->priority_ratio < x->priority_ratio) return -1;
    return 0;
}

/* Cota superior con relajación fraccional */
double compute_bound(Node *u) {
    if (u->weight >= MAX_WEIGHT || u->volume >= MAX_VOLUME) {
        return 0.0;
    }

    double bound = u->value;
    double total_weight = u->weight;
    double total_volume = u->volume;

    int j = u->level;

    while (j < N &&
           total_weight + packages[j].weight <= MAX_WEIGHT &&
           total_volume + packages[j].volume <= MAX_VOLUME) {

        total_weight += packages[j].weight;
        total_volume += packages[j].volume;
        bound += packages[j].value;
        j++;
    }

    if (j < N) {
        double remaining_weight = MAX_WEIGHT - total_weight;
        double remaining_volume = MAX_VOLUME - total_volume;

        double fraction_by_weight = remaining_weight / packages[j].weight;
        double fraction_by_volume = remaining_volume / packages[j].volume;

        double fraction = fraction_by_weight < fraction_by_volume
                        ? fraction_by_weight
                        : fraction_by_volume;

        if (fraction > 0.0) {
            bound += packages[j].value * fraction;
        }
    }

    return bound;
}

void try_update_best(Node *u) {
    if (u->value > best_value) {
        #pragma omp critical
        {
            if (u->value > best_value) {
                best_value = u->value;
                memcpy(best_taken, u->taken, sizeof(int) * N);
            }
        }
    }
}

void dfs_branch_bound(Node start) {
    Node stack[2048];
    int top = 0;

    stack[top++] = start;

    while (top > 0) {
        Node u = stack[--top];

        if (u.bound <= best_value) {
            continue;
        }

        if (u.level >= N) {
            try_update_best(&u);
            continue;
        }

        int idx = u.level;

        /* Rama 1: incluir paquete */
        Node with = u;
        with.level = idx + 1;
        with.weight += packages[idx].weight;
        with.volume += packages[idx].volume;
        with.value += packages[idx].value;
        with.taken[idx] = 1;

        if (with.weight <= MAX_WEIGHT && with.volume <= MAX_VOLUME) {
            try_update_best(&with);
            with.bound = compute_bound(&with);

            if (with.bound > best_value) {
                stack[top++] = with;
            }
        }

        /* Rama 2: excluir paquete */
        Node without = u;
        without.level = idx + 1;
        without.taken[idx] = 0;
        without.bound = compute_bound(&without);

        if (without.bound > best_value) {
            stack[top++] = without;
        }
    }
}

void generate_frontier(Frontier *frontier) {
    frontier->count = 0;

    Node root;
    root.level = 0;
    root.weight = 0.0;
    root.volume = 0.0;
    root.value = 0;
    memset(root.taken, 0, sizeof(root.taken));
    root.bound = compute_bound(&root);

    Node queue[1024];
    int qh = 0;
    int qt = 0;

    queue[qt++] = root;

    while (qh < qt) {
        Node u = queue[qh++];

        if (u.level >= INITIAL_DEPTH || u.level >= N) {
            frontier->nodes[frontier->count++] = u;
            continue;
        }

        int idx = u.level;

        /* Incluir paquete */
        Node with = u;
        with.level = idx + 1;
        with.weight += packages[idx].weight;
        with.volume += packages[idx].volume;
        with.value += packages[idx].value;
        with.taken[idx] = 1;

        if (with.weight <= MAX_WEIGHT && with.volume <= MAX_VOLUME) {
            with.bound = compute_bound(&with);
            if (with.bound > best_value) {
                queue[qt++] = with;
            }
        }

        /* Excluir paquete */
        Node without = u;
        without.level = idx + 1;
        without.taken[idx] = 0;
        without.bound = compute_bound(&without);

        if (without.bound > best_value) {
            queue[qt++] = without;
        }
    }
}

void print_solution() {
    double total_weight = 0.0;
    double total_volume = 0.0;
    double total_relative_use = 0.0;

    printf("\n=== Mejor solución encontrada ===\n");
    printf("Valor total: %d\n", best_value);

    printf("\nPaquetes seleccionados:\n");

    for (int i = 0; i < N; i++) {
        if (best_taken[i]) {
            printf(
                "Paquete ordenado=%d | id_original=%d | peso=%.2f kg | volumen=%.2f m3 | valor=%d | uso_relativo=%.4f | ratio_prioridad=%.2f\n",
                i,
                packages[i].id_original,
                packages[i].weight,
                packages[i].volume,
                packages[i].value,
                packages[i].relative_use,
                packages[i].priority_ratio
            );

            total_weight += packages[i].weight;
            total_volume += packages[i].volume;
            total_relative_use += packages[i].relative_use;
        }
    }

    printf("\nUso del container:\n");
    printf("Peso usado: %.2f / %.2f kg  (%.2f%%)\n",
           total_weight, MAX_WEIGHT, 100.0 * total_weight / MAX_WEIGHT);

    printf("Volumen usado: %.2f / %.2f m3 (%.2f%%)\n",
           total_volume, MAX_VOLUME, 100.0 * total_volume / MAX_VOLUME);

    printf("Uso relativo acumulado: %.4f\n", total_relative_use);
}

int main() {
    N = 10;

    MAX_WEIGHT = 1000.0;  // kg
    MAX_VOLUME = 60.0;    // m3

    double weights[] = {120, 300, 250, 100, 180, 400, 90, 220, 150, 350};
    double volumes[] = {8,   20,  12,  5,   15,  25,  3,  10,  7,   18};
    int values[]     = {80,  120, 150, 60,  100, 170, 55, 130, 90,  160};

    for (int i = 0; i < N; i++) {
        packages[i].id_original = i;
        packages[i].weight = weights[i];
        packages[i].volume = volumes[i];
        packages[i].value = values[i];

        packages[i].relative_use =
            (packages[i].weight / MAX_WEIGHT) +
            (packages[i].volume / MAX_VOLUME);

        packages[i].priority_ratio =
            packages[i].value / packages[i].relative_use;
    }

    qsort(packages, N, sizeof(Package), cmp_packages);

    printf("Paquetes ordenados por prioridad logística:\n");
    for (int i = 0; i < N; i++) {
        printf(
            "[%d] id_original=%d peso=%.2f volumen=%.2f valor=%d uso_relativo=%.4f ratio_prioridad=%.2f\n",
            i,
            packages[i].id_original,
            packages[i].weight,
            packages[i].volume,
            packages[i].value,
            packages[i].relative_use,
            packages[i].priority_ratio
        );
    }

    Frontier frontier;
    generate_frontier(&frontier);

    printf("\nSubárboles iniciales generados: %d\n", frontier.count);

    double t0 = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < frontier.count; i++) {
        int tid = omp_get_thread_num();

        printf(
            "Thread %d explora subárbol %d | level=%d | value=%d | weight=%.2f | volume=%.2f | bound=%.2f\n",
            tid,
            i,
            frontier.nodes[i].level,
            frontier.nodes[i].value,
            frontier.nodes[i].weight,
            frontier.nodes[i].volume,
            frontier.nodes[i].bound
        );

        dfs_branch_bound(frontier.nodes[i]);
    }

    double t1 = omp_get_wtime();

    print_solution();

    printf("\nTiempo paralelo: %.6f segundos\n", t1 - t0);
    printf("Threads disponibles: %d\n", omp_get_max_threads());

    return 0;
}