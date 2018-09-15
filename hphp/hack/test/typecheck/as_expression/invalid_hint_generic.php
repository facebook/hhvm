<?hh // strict

function foo<T>(mixed $x): void {
  $x as T;
  hh_show($x);
}
