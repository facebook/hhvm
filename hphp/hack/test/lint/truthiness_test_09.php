<?hh // strict

function test(dict<string, int> $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
