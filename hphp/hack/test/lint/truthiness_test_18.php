<?hh

function test(Pair<int, int> $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
