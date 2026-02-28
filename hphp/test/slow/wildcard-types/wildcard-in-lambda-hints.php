<?hh

<<__EntryPoint>>
function main():void {
  $f = (vec<_> $x) ==> $x[0];
  $g = (int $x) :vec<_> ==> vec[$x];

  $a = $f(vec[2]);
  var_dump($a);
  $b = $g(3);
  var_dump($b);
}
