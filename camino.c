#include <stdio.h>
#include <limits.h>
#include <omp.h>

#define N 5

int grid[N][N] = {
    {1, 3, 1, 2, 9},
    {7, 3, 4, 9, 9},
    {1, 7, 5, 5, 3},
    {2, 3, 4, 2, 5},
    {7, 7, 1, 3, 1}
};

int dx[4] = {-1, 1, 0, 0};
int dy[4] = {0, 0, -1, 1};

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
    if (costo_actual >= *mejor_costo) { // Restriccion
        return;
    }
    if (x == N - 1 && y == N - 1) { // Restriccion
        if (costo_actual < *mejor_costo) {
            *mejor_costo = costo_actual;
        }
        return;
    }
    for (i = 0; i < 4; i++) { // Avance
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (es_valido(nx, ny, visitado)) {
            visitado[nx][ny] = 1;
            backtracking_secuencial(nx, ny, costo_actual + grid[nx][ny], mejor_costo, visitado);
            visitado[nx][ny] = 0;
        }
    }
}

int backtracking_paralelo() {
    int mejor_costo_global = INT_MAX;
    int primeros_x[2] = {1, 0};
    int primeros_y[2] = {0, 1};
    int total_ramas = 2;
    #pragma omp parallel for shared(mejor_costo_global)
    for (int i = 0; i < total_ramas; i++) {
        int nx = primeros_x[i];
        int ny = primeros_y[i];
        if (nx < N && ny < N) {
            int visitado_local[N][N] = {0};
            int mejor_costo_local = INT_MAX;
            visitado_local[0][0] = 1;
            visitado_local[nx][ny] = 1;
            backtracking_secuencial(nx, ny, grid[0][0] + grid[nx][ny], &mejor_costo_local, visitado_local);
            #pragma omp critical
            {
                if (mejor_costo_local < mejor_costo_global) {
                    mejor_costo_global = mejor_costo_local;
                }
            }
        }
    }

    return mejor_costo_global;
}

int main() {
    omp_set_num_threads(4);
    int mejor_costo_sec = INT_MAX;
    int visitado[N][N] = {0};

    visitado[0][0] = 1;

    printf("=== Camino mas corto con Backtracking en C ===\n");

    double inicio_sec = omp_get_wtime();
    backtracking_secuencial(0, 0, grid[0][0], &mejor_costo_sec, visitado);
    double fin_sec = omp_get_wtime();

    double tiempo_sec = fin_sec - inicio_sec;

    double inicio_par = omp_get_wtime();
    int mejor_costo_par = backtracking_paralelo();
    double fin_par = omp_get_wtime();

    double tiempo_par = fin_par - inicio_par;

    printf("Mejor costo secuencial : %d\n", mejor_costo_sec);
    printf("Tiempo secuencial      : %.8f segundos\n", tiempo_sec);

    printf("Mejor costo paralelo   : %d\n", mejor_costo_par);
    printf("Tiempo paralelo        : %.8f segundos\n", tiempo_par);

    if (tiempo_par > 0.0) {
        printf("Speedup aproximado     : %.4f\n", tiempo_sec / tiempo_par);
    }

    return 0;
}