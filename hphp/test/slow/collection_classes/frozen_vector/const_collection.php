<?hh

// Test the ConstCollection interface.

function foo(ConstCollection $fv) {
  var_dump($fv->isEmpty());
  var_dump($fv->count());

  foreach ($fv as $e) {
    var_dump($e);
  }
}

function main() {
  $v = Vector {1, 2, 3};
  $fv = new FrozenVector($v);
  foo($fv);
}

main();
