<?hh

/*
 * This file was causing an infinite loop in an earlier implementation of
 * --color.
 */

class F {
  public function g() {
  }
}

function f() {
  return (new F())->g();
}
