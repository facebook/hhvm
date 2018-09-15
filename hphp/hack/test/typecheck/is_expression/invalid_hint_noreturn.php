<?hh // strict

function foo(mixed $x): void {
  if ($x is noreturn) {
    hh_show($x);
  }
}
