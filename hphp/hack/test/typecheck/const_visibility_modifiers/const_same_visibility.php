<?hh // strict

abstract class A {
  abstract protected const int ARGS;
}

abstract class B extends A {
  abstract public const int ARGS;
}

class C extends B {
  public const int ARGS = 5;
}
