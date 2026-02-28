<?hh

function f(bool $cond, string $key): void {
  $m = Map {};
  $m[$key] = Map {};
  $i = 1;
  $j = 2;
  $k = 3;
  $l = 4;
  $n = 5;
  while ($cond) {
    $a = $m->get($key) ?? Map {};
    $b = $a->get($key) ?? Map {};
    $c = $b->get($key) ?? Map {};

    /* force typechecking the body of the loop multiple times with this charming
     * little dependency chain */
    $i = $j;
    $j = $k;
    $k = $l;
    $l = $n;

    $c[$key] = 1;
    $b[$key] = $c;
    $a[$key] = $b;
  }
}
