<?hh

function foo(): int {
  $x = coroutine function() {
    return 5;
  };
  return $x();
}
