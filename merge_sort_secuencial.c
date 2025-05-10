#include <stdio.h>
#include <stdlib.h>
#include <time.h> // ⬅️ Para medir el tiempo

// Función para combinar dos subarreglos ordenados
void merge(int arr[], int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int *L = malloc(n1 * sizeof(int));
    int *R = malloc(n2 * sizeof(int));

    if (L == NULL || R == NULL) {
        printf("Error de asignación de memoria.\n");
        exit(1);
    }

    for (i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    i = 0; j = 0; k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j])
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1)
        arr[k++] = L[i++];
    while (j < n2)
        arr[k++] = R[j++];

    free(L);
    free(R);
}

// Merge Sort
void mergeSort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

// Imprimir arreglo
void printArray(int arr[], int size) {
    for (int i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

// main con argumentos
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <cantidad_de_elementos>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);

    if (n <= 0) {
        printf("El número debe ser mayor que 0.\n");
        return 1;
    }

    // Asignar memoria dinámicamente para el array
    int *arr = malloc(n * sizeof(int));
    if (arr == NULL) {
        printf("Error de asignación de memoria.\n");
        return 1;
    }

    // Inicializar la semilla para números aleatorios
    srand(time(NULL));

    // Llenar el arreglo con números aleatorios
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 10000 + 1; // Números entre 1 y 10000
    }

    // Medir el tiempo de ejecución
    clock_t inicio = clock();
    mergeSort(arr, 0, n - 1);
    clock_t fin = clock();
    
    double tiempo = (double)(fin - inicio) / CLOCKS_PER_SEC;
    printf("Tiempo de ejecución: %.6f segundos\n", tiempo);

    // Liberar la memoria asignada
    free(arr);

    return 0;
}
