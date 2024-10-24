#include <stdio.h>
#include <stdbool.h>

bool es_primo(int num) {
    if (num <= 1) {
        return false;
    }
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) {
            return false;
        }
    }
    return true;
}

int main() {
    for (int i = 1; i <= 12; i++) {
        if (es_primo(i)) {
            printf("%d es primo\n", i);
        } else {
            printf("%d no es primo\n", i);
        }
    }
    return 0;
}
