<?hh

function so_map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  throw new Exception();
}

<<__SupportDynamicType>>
function stack_overflow(): void {
  $t1 = tuple('a', $x ==> Shapes::idx($x, 'a'));
  $t2 = tuple('b', $x ==> Shapes::idx($x, 'b'));
  $v = vec[$t1, $t2];
  so_map(
    $v,
    $pair ==> {
      $p0 = $pair[0];
      $p1 = $pair[1];
      $p1($p0);},
  );
}
