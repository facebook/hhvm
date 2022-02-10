<?hh // strict

function test(int $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
