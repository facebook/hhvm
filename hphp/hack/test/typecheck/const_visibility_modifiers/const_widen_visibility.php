<?hh // strict

abstract class A {
 abstract protected const int ARGS;
}

class B extends A {
  public const int ARGS = 5;
}
