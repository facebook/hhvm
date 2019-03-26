<?hh

function update(&$ref, $val) {
  var_dump($ref); // 100
  $ref = $val;
}

function test() {
  $foo = new stdClass;
  $bar = new stdClass;
  $bar->baz = 100;
  $foo->bar = array($bar);
  $k = new stdClass;
  $k->key = 0;
  update(&$foo->bar[$k?->key]->baz, 200);
  var_dump($foo->bar[$k?->key]->baz); // 200
}


<<__EntryPoint>>
function main_nullsafe_prop_12() {
test();
}
