class Test {
  static void f()
  {
    System.out.println("f"); 
  }
  static int g(int x, int y)
  {
    System.out.println("g " + x + " " + y); 
    return x + y;
  }
  static int a;
  int b;
  int h() {
    System.out.println("h");
    return b;
  }
  static int k(Testcb cb, int x)
  {
    System.out.println("k " + x);
    cb.f();
    return cb.g(x); 
  }
}
