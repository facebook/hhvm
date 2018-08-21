<?hh // strict

coroutine function f(Container<int> $arr): int {
  // not ok - argument to suspend is a call to standard function
  $a = suspend array_filter($arr);
  return 1;
}
