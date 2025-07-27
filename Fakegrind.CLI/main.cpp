#include <iostream>

int main() {
    Sleep(5000);
    printf("hi\n");
    auto x = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * 6000));
    memset(x, 0xFF, sizeof(uint64_t) * 6000);
    for (int i = 0; i < 6000; i++)
        printf("%llu\n", x[i]);

    return 0;
}