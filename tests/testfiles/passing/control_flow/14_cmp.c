int main() {
  int x = 2;
  int y = 7;

  if (x >= y) {
    return 0;
  } else if (x == y) {
    return 1;
  }

  int z = 8;
  if (z + 2 < x + 2 * y) {
    return 2;
  }

  return z;
}
