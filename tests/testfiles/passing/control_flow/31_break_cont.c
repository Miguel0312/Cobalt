int main() {
    int x = 0;
    for (int i = 0; i < 5; i++) {
        if (x % 2 == 0) {
            continue;
        }
        x++;
    }

    return x;
}
