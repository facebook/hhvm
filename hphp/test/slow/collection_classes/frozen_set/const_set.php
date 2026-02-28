<?hh

// Check that ImmSet is a ConstSet.

function foo(ConstSet $s) :mixed{
  foreach ($s as $e) {
    var_dump($e);
  }
}

function main() :mixed{
  foo(new ImmSet(Vector {1, 2, 3}));
  foo(Set {4, 5, 6});
}


<<__EntryPoint>>
function main_const_set() :mixed{
main();
}
