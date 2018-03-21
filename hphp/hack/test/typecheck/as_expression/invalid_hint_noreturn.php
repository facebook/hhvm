<?hh // strict

function foo(mixed $x): void {
  $x as noreturn;
  hh_show($x);
}
