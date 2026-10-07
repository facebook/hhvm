<?hh

class A {
  public int $x = 0;
}

class B extends A {
  public readonly int $x = 0;
}
