<?hh // strict

function test(vec<int> $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
