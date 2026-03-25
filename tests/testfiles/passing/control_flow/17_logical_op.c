
int main() {
  int x = 3;
  int y = 1;
  int z = 0;

  if (x == y || x / 2 == y) {
    z = z + 1;
  }
  if (x == y && x / 2 == y) {
    z = z + 2;
  }

  return z;
}
