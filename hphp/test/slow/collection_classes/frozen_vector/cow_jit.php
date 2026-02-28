<?hh

// Test that the JIT triggers COW.

function main() :mixed{
  $v = Vector {1, 2, 3};
  $fv = $v->toImmVector();
  $v[0] = 10; // This is handled in the JIT, and should it trigger COW.
  var_dump($v);
  var_dump($fv);
}


<<__EntryPoint>>
function main_cow_jit() :mixed{
main();
}
