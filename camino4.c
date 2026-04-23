#include <stdio.h>
#include <limits.h>
#include <omp.h>

#define N 5
#define MAX_RAMAS 4

int grid[N][N] = {
    {1, 3, 1, 2, 9},
    {7, 3, 4, 9, 9},
    {1, 7, 5, 5, 3},
    {2, 3, 4, 2, 5},
    {7, 7, 1, 3, 1}
};

int dx[4] = {-1, 1, 0, 0};
int dy[4] = {0, 0, -1, 1};

typedef struct {
    int x;
    int y;
    int costo;
    int visitado[N][N];

    int x1, y1;
    int x2, y2;

    double tiempo;
    int mejor_costo_local;
    int hilo;
} Rama;

int es_valido(int x, int y, int visitado[N][N]) {
    return x >= 0 && x < N && y >= 0 && y < N && !visitado[x][y];
}

void copiar_matriz(int origen[N][N], int destino[N][N]) {
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            destino[i][j] = origen[i][j];
        }
    }
}

void backtracking_secuencial(int x, int y, int costo_actual, int *mejor_costo, int visitado[N][N]) {
    int i;
    if (costo_actual >= *mejor_costo) {
        return;
    }
    if (x == N - 1 && y == N - 1) {
        if (costo_actual < *mejor_costo) {
            *mejor_costo = costo_actual;
        }
        return;
    }
    for (i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (es_valido(nx, ny, visitado)) {
            visitado[nx][ny] = 1;
            backtracking_secuencial(nx, ny, costo_actual + grid[nx][ny], mejor_costo, visitado);
            visitado[nx][ny] = 0;
        }
    }
}

void generar_ramas_iniciales(Rama ramas[MAX_RAMAS], int *total_ramas) {
    int visitado0[N][N] = {0};
    int i, j;

    visitado0[0][0] = 1;
    *total_ramas = 0;

    for (i = 0; i < 4; i++) {
        int x1 = 0 + dx[i];
        int y1 = 0 + dy[i];

        if (es_valido(x1, y1, visitado0)) {
            int visitado1[N][N];
            int costo1;

            copiar_matriz(visitado0, visitado1);
            visitado1[x1][y1] = 1;
            costo1 = grid[0][0] + grid[x1][y1];

            for (j = 0; j < 4; j++) {
                int x2 = x1 + dx[j];
                int y2 = y1 + dy[j];

                if (es_valido(x2, y2, visitado1)) {
                    copiar_matriz(visitado1, ramas[*total_ramas].visitado);
                    ramas[*total_ramas].visitado[x2][y2] = 1;

                    ramas[*total_ramas].x = x2;
                    ramas[*total_ramas].y = y2;
                    ramas[*total_ramas].costo = costo1 + grid[x2][y2];

                    ramas[*total_ramas].x1 = x1;
                    ramas[*total_ramas].y1 = y1;
                    ramas[*total_ramas].x2 = x2;
                    ramas[*total_ramas].y2 = y2;

                    ramas[*total_ramas].tiempo = 0.0;
                    ramas[*total_ramas].mejor_costo_local = INT_MAX;
                    ramas[*total_ramas].hilo = -1;

                    (*total_ramas)++;

                    if (*total_ramas == MAX_RAMAS) {
                        return;
                    }
                }
            }
        }
    }
}

void imprimir_ramas(Rama ramas[MAX_RAMAS], int total_ramas) {
    int i;
    printf("=== Ramas iniciales generadas para el paralelo ===\n");
    for (i = 0; i < total_ramas; i++) {
        printf("Rama %d: (0,0) -> (%d,%d) -> (%d,%d), costo parcial = %d\n",
               i,
               ramas[i].x1, ramas[i].y1,
               ramas[i].x2, ramas[i].y2,
               ramas[i].costo);
    }
    printf("\n");
}

int backtracking_paralelo_4ramas(Rama ramas[MAX_RAMAS], int total_ramas) {
    int mejor_costo_global = INT_MAX;
    int i;
    #pragma omp parallel for shared(mejor_costo_global, ramas)
    for (i = 0; i < total_ramas; i++) {
        int mejor_costo_local = INT_MAX;
        int visitado_local[N][N];
        int hilo = omp_get_thread_num();
        double inicio_rama, fin_rama;
        copiar_matriz(ramas[i].visitado, visitado_local);
        inicio_rama = omp_get_wtime();
        backtracking_secuencial(
            ramas[i].x,
            ramas[i].y,
            ramas[i].costo,
            &mejor_costo_local,
            visitado_local
        );
        fin_rama = omp_get_wtime();
        ramas[i].tiempo = fin_rama - inicio_rama;
        ramas[i].mejor_costo_local = mejor_costo_local;
        ramas[i].hilo = hilo;

        #pragma omp critical
        {
            if (mejor_costo_local < mejor_costo_global) {
                mejor_costo_global = mejor_costo_local;
            }
        }
    }

    return mejor_costo_global;
}

void imprimir_resultados_ramas(Rama ramas[MAX_RAMAS], int total_ramas, double tiempo_sec_total) {
    int i;
    printf("=== Resultados por rama ===\n");
    for (i = 0; i < total_ramas; i++) {
        printf("Rama %d | Hilo %d | Ruta: (0,0)->(%d,%d)->(%d,%d) | Mejor costo local: %d | Tiempo: %.8f s",
               i,
               ramas[i].hilo,
               ramas[i].x1, ramas[i].y1,
               ramas[i].x2, ramas[i].y2,
               ramas[i].mejor_costo_local,
               ramas[i].tiempo);

        if (tiempo_sec_total > 0.0) {
            printf(" | Proporcion respecto secuencial total: %.4f",
                   ramas[i].tiempo / tiempo_sec_total);
        }

        printf("\n");
    }
    printf("\n");
}

int main() {
    int mejor_costo_sec = INT_MAX;
    int visitado[N][N] = {0};
    Rama ramas[MAX_RAMAS];
    int total_ramas = 0;

    double inicio_sec, fin_sec, tiempo_sec;
    double inicio_par, fin_par, tiempo_par;
    int mejor_costo_par;

    visitado[0][0] = 1;

    omp_set_num_threads(4);

    printf("=== Camino mas corto con Backtracking ===\n");

    inicio_sec = omp_get_wtime();
    backtracking_secuencial(0, 0, grid[0][0], &mejor_costo_sec, visitado);
    fin_sec = omp_get_wtime();
    tiempo_sec = fin_sec - inicio_sec;

    generar_ramas_iniciales(ramas, &total_ramas);
    //imprimir_ramas(ramas, total_ramas);

    inicio_par = omp_get_wtime();
    mejor_costo_par = backtracking_paralelo_4ramas(ramas, total_ramas);
    fin_par = omp_get_wtime();
    tiempo_par = fin_par - inicio_par;

    //imprimir_resultados_ramas(ramas, total_ramas, tiempo_sec);

    printf("=== Resultados globales ===\n");
    printf("Mejor costo secuencial : %d\n", mejor_costo_sec);
    printf("Tiempo secuencial total: %.8f s\n", tiempo_sec);

    printf("Mejor costo paralelo   : %d\n", mejor_costo_par);
    printf("Tiempo paralelo total  : %.8f s\n", tiempo_par);

    if (tiempo_par > 0.0) {
        printf("Speedup global         : %.4f\n", tiempo_sec / tiempo_par);
    }

    return 0;
}