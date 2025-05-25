#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

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

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

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
            lista[i] = rand() % 10000 + 1;
    }

    clock_t start = clock();

    int local_n = counts[rank];
    int *local_data = malloc(local_n * sizeof(int));

    MPI_Scatterv(lista, counts, displs, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Ordena localmente
    mergeSort(local_data, 0, local_n - 1);

    // Fusionar en etapas log2(size)
    int step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                // Recibir tamaño y datos
                int recv_size;
                MPI_Recv(&recv_size, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int *recv_data = malloc(recv_size * sizeof(int));
                MPI_Recv(recv_data, recv_size, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Fusionar
                int *merged = mergeArrays(local_data, local_n, recv_data, recv_size);
                free(local_data);
                free(recv_data);
                local_data = merged;
                local_n += recv_size;
            }
        } else {
            int target = rank - step;
            // Enviar tamaño y datos
            MPI_Send(&local_n, 1, MPI_INT, target, 0, MPI_COMM_WORLD);
            MPI_Send(local_data, local_n, MPI_INT, target, 0, MPI_COMM_WORLD);
            free(local_data);
            break; // proceso sale
        }
        step *= 2;
    }

    if (rank == 0) {
        free(local_data);
        free(lista);
    }

    free(counts);
    free(displs);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    if (rank == 0) {
        printf("Tiempo total: %f segundos\n", time_spent);
    }

    MPI_Finalize();
    return 0;
}
