<?hh

function test() {
  $x = 42;
  $a = array(array('x' => 24));
  array_map('extract', $a);
  return $x;
}

var_dump(test());
