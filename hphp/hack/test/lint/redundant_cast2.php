<?hh

class C {}
interface I {}

function not_redundant(C $c, mixed $m, I $i): void {
  (string) $c;
  (float) $c;
  (bool) $c;
  (int) $c;

  (string) $m;
  (float) $m;
  (bool) $m;
  (int) $m;

  (string) $i;
  (float) $i;
  (bool) $i;
  (int) $i;
}
