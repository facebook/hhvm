<?hh

abstract class C {
  <<__LateInit>>
  public int $foo;
}

final class D extends C {
  public int $foo = 1;
}
