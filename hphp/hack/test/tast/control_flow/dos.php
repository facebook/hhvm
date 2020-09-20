<?hh //strict

/* test that there are no interference between the Do continuations of multiple
 * do loops */
function f(bool $b, int $a): void {
  do {
    $a; // int | string
    $a = "";
  } while ($b);
  $a; // string

  do {
    $a; // string
    $a = "";
  } while ($b);

  do {
    $a; // string
    $a = 0;
    do {
      $a; // int
      $a = 1;
    } while ($b);
    $a = "";
  } while ($b);
}
