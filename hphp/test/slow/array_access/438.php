<?hh

function test($x) {
  $a = $x;
  $b = $a;
  $a[0] = new stdClass();
  $a[0]->foo = 1;
  var_dump($a, $b);
  $a = $x;
  $b = $a;
  $a[0] = darray[];
  $a[0][1] = 1;
  var_dump($a, $b);
  }

<<__EntryPoint>>
function main_438() {
test(varray[false]);
var_dump(varray[false]);
}
