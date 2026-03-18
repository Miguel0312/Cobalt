int main() {
  int y = 7;
  {
    int x = 5;
    int x = 3;
    {
    }
  }

  return y;
}
