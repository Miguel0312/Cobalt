int main() {
  int x = 0;
  if (1 + 2 && 2 * 1 || (1 == 2)) {
    x = x + 1;
  }
  if ((1 + 2 && 2 * 1) || (1 == 2)) {
    x = x + 2;
  }

  return x;
}
