<?hh

<<__EntryPoint>>
function memory_leak_test_entry_point(): void {

  ini_set('hhvm.enable_gc', 1);
  ini_set('hhvm.eager_gc', 1);
  ini_set('memory_limit', '3M');

  $dict = dict['one' => 'apple', 'two' => 'pear', 'three' => 'tangerine'];

  for ($i = 1; $i <= 50000; $i++) {
    is_keyed_container($dict);
  }
  echo "ok\n";
}

function is_keyed_container(mixed $item_to_test): bool {
  return $item_to_test is KeyedContainer<_, _>;
}
