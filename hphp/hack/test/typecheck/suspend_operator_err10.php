<?hh // strict

function f(): int {
  // not ok - call to coroutine outside the suspend
  $a = (coroutine () ==> 1)();
  return 1;
}
