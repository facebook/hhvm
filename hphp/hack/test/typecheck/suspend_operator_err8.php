<?hh // strict

function f((coroutine function(): int) $v): int {
  // not ok - call to coroutine outside the suspend
  $a = $v();
  return 1;
}
