<?hh

coroutine function foo(): void {
  if (suspend    bar1()) {
    $x = suspend
      bar2() + suspend           bar2();
    return $x;
  }

  return suspend
  bar3();
}
