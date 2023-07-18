<?hh

<<__EntryPoint>>
function main():void {
  $g = (int $x) :vec<_> ==> 3;

  $b = $g(3);
  var_dump($b);
}
