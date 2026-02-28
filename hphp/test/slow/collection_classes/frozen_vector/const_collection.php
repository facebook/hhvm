<?hh

// Test the ConstCollection interface.

function foo(ConstCollection $fv) :mixed{
  var_dump($fv->isEmpty());
  var_dump($fv->count());

  foreach ($fv as $e) {
    var_dump($e);
  }
}

function main() :mixed{
  $v = Vector {1, 2, 3};
  $fv = new ImmVector($v);
  foo($fv);
}


<<__EntryPoint>>
function main_const_collection() :mixed{
main();
}
