<?hh

function f(inout $a, inout $b) :mixed{
  $a[0] = 1;
  $b[1] = 2;
  return 3;
}





<<__EntryPoint>>
function test() :mixed{
  $a = dict[];
  f(inout $a, inout $a);
  var_dump($a);
  $a = dict[];
  $a[100] = f(inout $a, inout $a);
  var_dump($a);



}
