<?hh

function foo<T>(mixed $x): void {
  if ($x is T) {
    hh_show($x);
  }
}
