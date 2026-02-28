<?hh

abstract class C {
  public int $foo;
}

final class D extends C {
  <<__LateInit>>
  public int $foo;
}
