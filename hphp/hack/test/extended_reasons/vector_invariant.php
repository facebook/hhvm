<?hh

function foo(): Vector<int> {
  $x =  Vector { 1 , '2' };
  $y = $x;
  $z = $y;
  // hh_force_solve();
  return $z;
}
