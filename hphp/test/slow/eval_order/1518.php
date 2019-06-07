<?hh

function f(&$a, &$b) {
  $a[0] = 1;
  $b[1] = 2;
  return 3;
}





<<__EntryPoint>>
function test() {
  $a = array();
  f(&$a, &$a);
  var_dump($a);
  $a = array();
  $a[100] = f(&$a, &$a);
  var_dump($a);



}
