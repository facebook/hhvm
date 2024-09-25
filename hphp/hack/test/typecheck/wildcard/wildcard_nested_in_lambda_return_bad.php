<?hh

function test1():string {
  $f = (int $_) : vec<_> ==> vec[3];
  $x = $f(3);
  return $x[0];
}
