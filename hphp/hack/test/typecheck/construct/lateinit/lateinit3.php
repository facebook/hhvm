<?hh // strict

abstract class A {
  <<__LateInit>>
  protected string $prop1;
  <<__LateInit>>
  protected static string $prop2;
}

final class C extends A {
}
