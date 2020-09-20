<?hh

function f(inout $a, inout $b) {
  $a[0] = 1;
  $b[1] = 2;
  return 3;
}





<<__EntryPoint>>
function test() {
  $a = darray[];
  f(inout $a, inout $a);
  var_dump($a);
  $a = darray[];
  $a[100] = f(inout $a, inout $a);
  var_dump($a);



}
