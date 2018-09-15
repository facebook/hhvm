<?hh // strict

function foo(mixed $x): void {
  if ($x is void) {
    hh_show($x);
  }
}
