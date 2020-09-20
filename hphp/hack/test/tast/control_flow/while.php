<?hh //strict

function f(bool $b, int $a): void {
  while ($b) {
    $a; // int | string
    $a = "";
  }
  $a; // int | string
}
