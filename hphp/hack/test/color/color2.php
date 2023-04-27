<?hh

/*
 * This file was causing an infinite loop in an earlier implementation of
 * --color.
 */

class F {
  public function g():dynamic {
    return 3;
  }
}

function f():dynamic {
  return (new F())->g();
}
