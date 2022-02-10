<?hh // strict

function test(bool $b, string $s): void {
  $x = $b ? $s : $b;
  if ($x) {
    do_something();
  }
}

function do_something(): void {}
