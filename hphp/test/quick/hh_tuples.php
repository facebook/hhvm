<?hh
function cons<X,Y>(X $x, Y $y) : (X,Y) {
  return vec[$x, $y];
}
function car<X,Y>((X,Y) $x): X {
  return $x[0];
}
function cdr<X,Y>((X,Y) $x): Y {
  return $x[1];
}
<<__EntryPoint>> function main(): void {
echo "1..2\n";
$v = cons("ok 1\n", "ok 2\n");
echo car($v), cdr($v);
}
