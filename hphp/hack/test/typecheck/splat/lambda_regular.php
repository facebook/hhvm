<?hh

function f((int, float, string) $t): void {
  $f = ($a, $b) ==> { throw new Exception(); };
  $f(...$t);
}
