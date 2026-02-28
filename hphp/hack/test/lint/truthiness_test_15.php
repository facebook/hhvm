<?hh

function test(Iterable<int> $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
