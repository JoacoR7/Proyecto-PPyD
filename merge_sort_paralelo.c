#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>

// Fusiona dos listas ordenadas
int* mergeArrays(int *a, int a_size, int *b, int b_size) {
    int *result = malloc((a_size + b_size) * sizeof(int));
    int i = 0, j = 0, k = 0;
    while (i < a_size && j < b_size) {
        if (a[i] <= b[j]) result[k++] = a[i++];
        else result[k++] = b[j++];
    }
    while (i < a_size) result[k++] = a[i++];
    while (j < b_size) result[k++] = b[j++];
    return result;
}

void mergeSort(int arr[], int left, int right) {
    if (left < right) {
        int mid = (left + right) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid+1, right);
        // merge ya está implementado, pero no es necesario aquí
        int *temp = mergeArrays(&arr[left], mid - left + 1, &arr[mid+1], right - mid);
        for (int i = 0; i < right - left + 1; i++) arr[left + i] = temp[i];
        free(temp);
    }
}

// Función para verificar si un número es primo
bool esPrimo(int num) {
    if (num < 2) return false;
    if (num == 2) return true;
    if (num % 2 == 0) return false;

    for (int i = 3; i < num; i ++) {
        if (num % i == 0) return false;
    }
    return true;
}

// Función que cuenta los primos en un arreglo
int contarPrimos(int arr[], int n) {
    int contador = 0;
    for (int i = 0; i < n; i++) {
        if (esPrimo(arr[i])) {
            contador++;
        }
    }
    return contador;
}

// ... [los includes y funciones auxiliares no cambian]

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double tiempo_rank0 = 0.0;
    clock_t start_rank0, end_rank0;

    if (rank == 0) {
        start_rank0 = clock();  // Inicio de tareas exclusivas de rank 0
    }

    if (argc != 2) {
        if (rank == 0) printf("Uso: %s <cantidad_de_elementos>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        if (rank == 0) printf("El número debe ser mayor que 0.\n");
        MPI_Finalize();
        return 1;
    }

    int *lista = NULL;
    int *counts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));

    int base = n / size;
    int resto = n % size;

    for (int i = 0; i < size; i++) {
        counts[i] = base + (i < resto ? 1 : 0);
        displs[i] = (i == 0) ? 0 : displs[i - 1] + counts[i - 1];
    }

    if (rank == 0) {
        lista = malloc(n * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < n; i++)
            lista[i] = rand() % 100000 + 1;
    }


    clock_t start_total = clock();

    int local_n = counts[rank];
    int *local_data = malloc(local_n * sizeof(int));

    MPI_Scatterv(lista, counts, displs, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Medición de tiempo de cómputo (primos + ordenamiento)
    clock_t start_comp = clock();

    int cantidadPrimos = contarPrimos(local_data, local_n);
    mergeSort(local_data, 0, local_n - 1);

    clock_t end_comp = clock();
    double tiempo_local = (double)(end_comp - start_comp) / CLOCKS_PER_SEC;

    // Recolección de tiempos de cómputo
    double *tiempos = NULL;
    if (rank == 0) {
        tiempos = malloc(size * sizeof(double));
    }
    MPI_Gather(&tiempo_local, 1, MPI_DOUBLE, tiempos, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Fusión paralela
    int step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                int recv_size;
                MPI_Recv(&recv_size, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int *recv_data = malloc(recv_size * sizeof(int));
                MPI_Recv(recv_data, recv_size, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                int *merged = mergeArrays(local_data, local_n, recv_data, recv_size);
                free(local_data);
                free(recv_data);
                local_data = merged;
                local_n += recv_size;
            }
        } else {
            int target = rank - step;
            MPI_Send(&local_n, 1, MPI_INT, target, 0, MPI_COMM_WORLD);
            MPI_Send(local_data, local_n, MPI_INT, target, 0, MPI_COMM_WORLD);
            free(local_data);
            break;
        }
        step *= 2;
    }

    if (rank == 0) {
        free(local_data);
        free(lista);
    }

    free(counts);
    free(displs);

    clock_t end_total = clock();
    double time_spent = (double)(end_total - start_total) / CLOCKS_PER_SEC;

    if (rank == 0) {
        // Balanceo de carga
        double min = tiempos[0], max = tiempos[0], sum = tiempos[0];
        for (int i = 1; i < size; i++) {
            if (tiempos[i] < min) min = tiempos[i];
            if (tiempos[i] > max) max = tiempos[i];
            sum += tiempos[i];
        }
        double promedio = sum / size;
        double desbalanceo = (max - min) / promedio;

        printf("Tiempo total: %f segundos\n", time_spent);
        printf("Balanceo de carga (tiempo de cómputo por proceso):\n");
        for (int i = 0; i < size; i++) {
            printf("  Proceso %d: %.6f segundos\n", i, tiempos[i]);
        }
        printf("  Máximo: %.6f, Mínimo: %.6f, Promedio: %.6f\n", max, min, promedio);


        free(tiempos);
        end_rank0 = clock();  // Fin de tareas exclusivas de rank 0
        tiempo_rank0 = (double)(end_rank0 - start_rank0) / CLOCKS_PER_SEC;
        printf("Tiempo exclusivo del proceso 0: %f segundos\n", tiempo_rank0);
    }

    MPI_Finalize();
    return 0;
}
