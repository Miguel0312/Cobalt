int main() {
    int x = 0;
    for (int i = 0; i < 5; i++) {
        x++;
        if (x % 2 == 0) {
            break;
        }
        x = x + 2;
    }

    return x;
}
