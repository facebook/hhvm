<?hh

// Test that COW makes materializing an ImmSet an O(1) operation.

function main() :mixed{
  $s = Set {};
  for ($i = 0; $i < 1000000; $i++) $s[] = $i + 42;
  for ($i = 0; $i < 10000; $i++) $s->toImmSet();

  var_dump($s->firstValue());
}


<<__EntryPoint>>
function main_cow_large() :mixed{
main();
}
