<?hh // strict

$z = new stdClass();
$z->m = 1;
function something(int $x): stdClass { global $z; return $z; }

function test(): void {
  global $z;
  $foo = null;
  $x =& something($foo?->bar); // ok
  $x->n = 2;
  var_dump($z);
}

test();
