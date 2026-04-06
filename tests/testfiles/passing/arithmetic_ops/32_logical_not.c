int main() {
    int x = 1;
    int y = !x;
    x = 0;
    y = y + !x;
    return y;
}
