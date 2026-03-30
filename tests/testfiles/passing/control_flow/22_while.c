int main() {
    int x = 10;
    int y;

    while ((y = x / 2)) {
        x = x - 1;
    }

    return x;
}
