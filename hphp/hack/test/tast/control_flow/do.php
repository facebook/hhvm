<?hh

function f(bool $b, int $a): void {
  do {
    $a; // int | string
    $a = "";
  } while ($b);
  $a; // string
}
