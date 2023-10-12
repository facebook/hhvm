<?hh // strict

abstract class A {
  <<__LateInit>>
  protected string $prop1;
  <<__LateInit>>
  protected static string $prop2;
  <<__LateInit>>
  private string $prop3;
  <<__LateInit>>
  private static string $prop4;
}

final class C extends A {
}
