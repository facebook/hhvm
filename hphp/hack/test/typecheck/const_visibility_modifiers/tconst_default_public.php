<?hh // strict

abstract class A {
  public abstract const type T1;
}
class B extends A {
  const type T1 = string;

}
