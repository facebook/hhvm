<?hh // strict

function test(Traversable<int> $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
