<?hh // strict

function foo(mixed $x): void {
  $x as void;
  hh_show($x);
}
