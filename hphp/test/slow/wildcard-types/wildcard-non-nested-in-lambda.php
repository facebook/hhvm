<?hh

<<__EntryPoint>>
function main():void {
  $f = (_ $x) ==> $x;
  $g = (int $x) : _ ==> vec[$x];
  $h = (int $x) : _ ==> null;

  $a = $f(vec[2]);
  $b = $f(null);
  var_dump($a);
  var_dump($b);
  $c = $g(3);
  var_dump($c);
  $d = $h(4);
  var_dump($d);
}
