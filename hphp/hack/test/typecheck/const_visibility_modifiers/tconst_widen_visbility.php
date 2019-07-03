<?hh // strict

abstract class A {
  abstract protected const type T;
}
class B extends A {
  public const type T = string;

}
