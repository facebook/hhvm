<?hh // strict

function test((bool, bool) $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
