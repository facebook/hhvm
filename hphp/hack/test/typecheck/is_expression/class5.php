<?hh // strict

interface I<Ta, Tb as Ta> {}

function f1(mixed $x): void {
  if ($x is I<_, _>) {
    hh_show($x);
  }
}
