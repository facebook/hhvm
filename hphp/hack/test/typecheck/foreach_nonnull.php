<?hh

function foo(vec<int> $v): void {
  $w = Vector{null, $v};
  $x = $w[0];
  if ($x !== null) {
    foreach ($x as $y) {}
  }
}
