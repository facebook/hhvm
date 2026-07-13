<?hh

function f($a, $b, $c) :mixed{
 return 'hello';
 }
function test($a) :mixed{
  $t1 = $b; $b++; $t2 = $b; $b++; $t3 = $b; $b++;
  $a->foo = f($t1, $t2, $t3);
  $x = $a->foo . f(1,2,3);
  return $x;
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
