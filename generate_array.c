#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

    // Asignar memoria dinámicamente
    int *arr = malloc(n * sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "Error al asignar memoria.\n");
        return 1;
    }

    srand(time(NULL));

    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 10000 + 1; // Números entre 1 y 10000
        printf("%d", arr[i]);
        if (i != n - 1) printf(" ");
    }

    printf("\n");

    // Liberar la memoria
    free(arr);

    return 0;
}
