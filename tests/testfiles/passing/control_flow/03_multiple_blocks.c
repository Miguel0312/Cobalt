int main() {
  int x = 3;
  {
    int x = 5;
    x = x + 2;
    return x;
  }

  return x;
}
