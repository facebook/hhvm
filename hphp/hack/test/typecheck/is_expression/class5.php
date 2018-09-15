<?hh // strict

interface I<Ta, Tb as Ta> {}

function f1(mixed $x): void {
  if ($x is I<_, _>) {
    hh_show($x);
  }
}

function f2(mixed $x): void {
  if ($x instanceof I) {
    hh_show($x);
  }
}
