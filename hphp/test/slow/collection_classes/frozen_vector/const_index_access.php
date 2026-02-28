<?hh

// Test the ConstIndexAccess interface


function foo(ConstIndexAccess $fv) :mixed{
  var_dump($fv->containsKey(0));
  var_dump($fv->containsKey(-1));
  var_dump($fv->containsKey(3));
  echo "---\n";

  var_dump($fv->get(0));
  var_dump($fv->get(2));
  var_dump($fv->get(-1));
  var_dump($fv->get(3));
  echo "---\n";

  var_dump($fv->at(0));
  var_dump($fv->at(2));
  var_dump($fv->at(3));
}

function main() :mixed{
  $v = Vector {1, 2, 3};
  $fv = new ImmVector($v);
  foo($fv);
}


<<__EntryPoint>>
function main_const_index_access() :mixed{
main();
}
