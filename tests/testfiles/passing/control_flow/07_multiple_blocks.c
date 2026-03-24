int main() {
  int x = 3;
  {
    int x = 2;
  }

  int x = 1;

  return x;
}
