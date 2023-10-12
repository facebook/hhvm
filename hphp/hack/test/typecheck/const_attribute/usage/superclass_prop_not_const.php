<?hh // strict

class A {
  public int $x = 4;
}

class B extends A {
  <<__Const>>
  public int $x = 42;
}
