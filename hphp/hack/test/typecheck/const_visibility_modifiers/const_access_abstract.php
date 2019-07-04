<?hh // strict

abstract class A {
  protected const int MAX_ARGS = 5;
}
class B extends A {
  public function get_int():int{
    return A::MAX_ARGS;  // ok
  }
}
class C {
  public function get_int():int{
    return A::MAX_ARGS;  // illegal
  }
}
