<?hh // strict

function test(array<int> $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
