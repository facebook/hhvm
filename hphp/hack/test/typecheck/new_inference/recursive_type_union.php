<?hh // strict

function test(bool $b): ?int {
  $x = new Vector(null); // new tvar v, $x : Vector<v>
  $y = ($b ? 0 : ($b? null : $x[0])); // $y : ?(int | v)
  $x[] = $y; // ?(int | v) <: v, should be simplified to ?int <: v
  return $x[0];
}
