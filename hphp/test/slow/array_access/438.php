<?hh

function test($x) :mixed{
  $a = $x;
  $b = $a;
  $a[0] = new stdClass();
  $a[0]->foo = 1;
  var_dump($a, $b);
  $a = $x;
  $b = $a;
  $a[0] = dict[];
  $a[0][1] = 1;
  var_dump($a, $b);
  }

<<__EntryPoint>>
function main_438() :mixed{
test(vec[false]);
var_dump(vec[false]);
}
