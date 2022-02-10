<?hh // strict

function test(Container<int> $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
