<?hh // strict

$z = new stdClass;
$z->m = 1;
function something($x) { global $z; return $z; }

function test() {
  global $z;
  $foo = null;
  $x =& something($foo?->bar); // ok
  $x->n = 2;
  var_dump($z);
}

test();
