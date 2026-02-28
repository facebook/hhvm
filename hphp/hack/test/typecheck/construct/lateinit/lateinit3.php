<?hh

abstract class A {
  <<__LateInit>>
  protected int $prop1;
  <<__LateInit>>
  protected static int $prop2;
  <<__LateInit>>
  private int $prop3;
  <<__LateInit>>
  private static int $prop4;
}

final class C extends A {
}
