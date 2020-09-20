<?hh

function foo($a) {
  $v = varray[];
  for ($i = 0; $i < 1024; $i++) {
    $v[] = $a;
  }
  return $v;
}

function test() {
  $a = foo(1);
  $a = foo($a);
  $a = foo($a);
  print_r($a, true);
}


<<__EntryPoint>>
function main_serialization_limit() {
test();
}
