#include <stdio.h>
#include "dummy_main.h"

int sum(int a, int b) {
    return a+b;
}

int main(int argc, char **argv) {
    int num1 = 2;
    int num2 = 3;
    int result = sum(num1,num2);
    printf("result %d", result);
    return 0;
}