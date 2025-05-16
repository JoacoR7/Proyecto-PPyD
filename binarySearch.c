#include "binarySearch.h"

int binarySearch(int x, int A[], int p, int r) {
    int low = p;
    int high = MAX(p, r + 1);
    while (low < high) {
        int mid = (low + high) / 2;
        if (x <= A[mid]) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    return high;
}
