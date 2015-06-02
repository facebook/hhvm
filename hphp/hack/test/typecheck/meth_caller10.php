<?hh // strict

class X<T> {
  public function foo(T $x): T {
    return $x;
  }
}

function test(): X<int> {
  $caller = meth_caller('X', 'foo');
  $x = new X();
  $caller($x, 'a string');
  return $x;
}
