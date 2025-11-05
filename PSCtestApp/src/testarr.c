#include <stdio.h>

void fillArray(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = i * 10;
    }
}

int main() {
    int data[5];
    fillArray(data, 5);

    for (int i = 0; i < 5; i++) {
        printf("%d ", data[i]);
    }
    return 0;
}
