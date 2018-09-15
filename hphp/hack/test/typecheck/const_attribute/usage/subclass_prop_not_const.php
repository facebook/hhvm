<?hh // strict

class A {
  <<__Const>>
  public int $x = 4;
}

class B extends A {
  public int $x = 42;
}
