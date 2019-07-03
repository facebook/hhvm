<?hh // strict

abstract class A {
  abstract public const type T;
}

class B extends A {
  private const type T = int;
}
