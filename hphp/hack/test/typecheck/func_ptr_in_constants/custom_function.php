<?hh

function foo(): int { return 42; }

class A {
  public static int $arr = foo();
}

abstract class B {
  const int const_arr = foo();
}
