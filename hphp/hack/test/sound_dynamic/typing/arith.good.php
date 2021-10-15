<?hh

enum E : int as int {
  C1 = 1;
}

function f(num $n, int $i, float $f, E $e, dynamic $d) : void {
  $x1 = $n + $d;
  $x2 = $i + $d;
  $x3 = $f + $d;
  $x4 = $e + $d;
  $x5 = $d + $d;
}
