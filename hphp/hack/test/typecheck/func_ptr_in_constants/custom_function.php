<?hh // partial

function foo() {}

class A {
  public static $arr = foo();
}

abstract class B {
  const const_arr = foo();
}
