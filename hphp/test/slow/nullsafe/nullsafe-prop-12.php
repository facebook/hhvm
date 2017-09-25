<?hh

function test() {
  $foo = new stdClass;
  $bar = new stdClass;
  $bar->baz = 100;
  $foo->bar = array($bar);
  $k = new stdClass;
  $k->key = 0;
  $x =& $foo->bar[$k?->key]->baz;
  var_dump($x); // 100
  $x = 200;
  var_dump($foo->bar[$k?->key]->baz); // 200
}

test();
