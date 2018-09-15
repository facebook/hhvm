<?hh

function test() {
  $x = 42;
  $a = array(array('x' => 24));
  array_walk(&$a, 'extract');
  return $x;
}

var_dump(test());
