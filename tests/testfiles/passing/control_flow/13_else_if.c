int main() {
  int x = 2;

  if (x - 2) {
    x = x + 1;
  } else if (x * 0) {
    x = x + 2;
  } else if (x / 10) {
    x = x + 2;
  } else {
    x = x + 3;
  }

  x = x + 4;
  return x;
}
