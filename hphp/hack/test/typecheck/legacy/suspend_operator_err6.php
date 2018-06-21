<?hh // strict

coroutine function f(shape('a' => int) $v): int {
  // not ok - argument to suspend is a call to standard function
  $a = suspend Shapes::idx($v, 'a');
  return 1;
}
