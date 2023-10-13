<?hh

function test(bool $b): int {
  $x = new Vector(null); // new tvar v, $x : Vector<v>
  $y = ($b? null : $x[0]); // $y : ?v
  $x[] = $y; // ?v <: v, should be simplified to Tnull <: v
  $x[] = 0;
  return $x[0]; // error, this is ?int
}
