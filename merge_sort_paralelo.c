#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>

///////////////////////////////////////////
// FUNCIONES AUXILIARES
///////////////////////////////////////////

/**
 * Fusiona dos arreglos ordenados en uno nuevo también ordenado.
 * Usado durante la fase de recombinación paralela y en el merge sort.
 */
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
/**
 * Implementación recursiva del algoritmo MergeSort.
 * Ordena un subarreglo de arr[] entre los índices left y right.
 */
void mergeSort(int arr[], int left, int right) {
    if (left < right) {
        int mid = (left + right) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid+1, right);
        // Se fusionan las mitades ordenadas usando mergeArrays.
        int *temp = mergeArrays(&arr[left], mid - left + 1, &arr[mid+1], right - mid);
        for (int i = 0; i < right - left + 1; i++) arr[left + i] = temp[i];
        free(temp);
    }
}

/**
 * Verifica si un número es primo.
 */
bool esPrimo(int num) {
    if (num < 2) return false;
    if (num == 2) return true;
    if (num % 2 == 0) return false;

    for (int i = 3; i < num; i ++) {
        if (num % i == 0) return false;
    }
    return true;
}

/**
 * Cuenta cuántos elementos del arreglo son números primos.
 */
int contarPrimos(int arr[], int n) {
    int contador = 0;
    for (int i = 0; i < n; i++) {
        if (esPrimo(arr[i])) {
            contador++;
        }
    }
    return contador;
}

///////////////////////////////////////////
// FUNCIÓN PRINCIPAL
///////////////////////////////////////////
int main(int argc, char *argv[]) {
    int rank, size;
    
    // Inicialización de MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double tiempo_rank0 = 0.0;
    clock_t start_rank0, end_rank0;

    if (rank == 0) {
        start_rank0 = clock();  // Medición de tiempo exclusivo de rank 0
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

    // Inicialización de estructuras para distribuir datos
    int *lista = NULL;
    int *counts = malloc(size * sizeof(int)); // Cantidad de elementos por proceso
    int *displs = malloc(size * sizeof(int)); // Desplazamientos en el arreglo original

    // Cálculo de particiones equitativas
    int base = n / size;
    int resto = n % size;

    for (int i = 0; i < size; i++) {
        counts[i] = base + (i < resto ? 1 : 0); // Se reparten los "restos" a los primeros procesos
        displs[i] = (i == 0) ? 0 : displs[i - 1] + counts[i - 1];
    }

    // Inicialización del arreglo aleatorio solo en el proceso 0
    if (rank == 0) {
        lista = malloc(n * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < n; i++)
            lista[i] = rand() % 100000 + 1;
    }

    // Tiempo total de ejecución desde aquí
    clock_t start_total = clock();

    // Reserva del arreglo local para cada proceso
    int local_n = counts[rank];
    int *local_data = malloc(local_n * sizeof(int));

    // Distribución de datos a todos los procesos
    MPI_Scatterv(lista, counts, displs, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Tiempo de inicio del cómputo local (primos + ordenamiento)
    clock_t start_comp = clock();

    // Conteo de primos y ordenamiento local
    int cantidadPrimos = contarPrimos(local_data, local_n);
    mergeSort(local_data, 0, local_n - 1);

    // Tiempo de fin de cómputo local
    clock_t end_comp = clock();
    double tiempo_local = (double)(end_comp - start_comp) / CLOCKS_PER_SEC;

    // Recolección de los tiempos de cómputo en el proceso 0
    double *tiempos = NULL;
    if (rank == 0) {
        tiempos = malloc(size * sizeof(double));
    }
    MPI_Gather(&tiempo_local, 1, MPI_DOUBLE, tiempos, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    /////////////////////////////////////////////////////////
    // RECOMBINACIÓN PARALELA
    /////////////////////////////////////////////////////////
    int step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                int recv_size;
                // Recibe tamaño y datos del vecino
                MPI_Recv(&recv_size, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int *recv_data = malloc(recv_size * sizeof(int));
                MPI_Recv(recv_data, recv_size, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Fusión local con los datos recibidos
                int *merged = mergeArrays(local_data, local_n, recv_data, recv_size);
                free(local_data);
                free(recv_data);
                local_data = merged;
                local_n += recv_size;
            }
        } else {
            // Envía su bloque al proceso que le corresponde
            int target = rank - step;
            MPI_Send(&local_n, 1, MPI_INT, target, 0, MPI_COMM_WORLD);
            MPI_Send(local_data, local_n, MPI_INT, target, 0, MPI_COMM_WORLD);
            free(local_data);
            break; // Sale del ciclo; ya no participa en siguientes pasos
        }
        step *= 2;
    }

    // Limpieza en proceso 0
    if (rank == 0) {
        free(local_data);
        free(lista);
    }

    free(counts);
    free(displs);

    // Fin del tiempo total
    clock_t end_total = clock();
    double time_spent = (double)(end_total - start_total) / CLOCKS_PER_SEC;

    ///////////////////////////////////////////
    // ANÁLISIS DE BALANCEO DE CARGA (Rank 0)
    ///////////////////////////////////////////
    if (rank == 0) {
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
