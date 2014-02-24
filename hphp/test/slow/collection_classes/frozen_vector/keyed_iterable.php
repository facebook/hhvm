<?hh

// Test the KeyedIterable interface.

function printFv($fv) {
  var_dump(get_class($fv));
  foreach ($fv as $e) {
    var_dump($e);
  }
}

function foo(KeyedIterable $fv) {
  echo "\nmap...\n";
  printFv($fv->map(function ($v) { return 2 * $v; }));

  echo "\nmapWithKey...\n";
  printFv($fv->mapWithKey(function ($k, $v) { return Pair {$k, $v}; }));

  echo "\nfilter...\n";
  printFv($fv->filter(function ($v) { return $v <= 1; }));

  echo "\nfilterWithKey...\n";
  printFv($fv->filterWithKey(function ($k, $v) { return $k + $v <= 3; }));

  echo "\nzip...\n";
  printFv($fv->zip(Vector {4, 5}));

  echo "\nkvzip...\n";
  printFv($fv->kvzip());

  echo "\nkeys...\n";
  printFv($fv->keys());

  echo "\ngetIterator...\n";
  $it = $fv->getIterator();
  while ($it->valid()) {
    echo "k = ";
    var_dump($it->key());
    echo "v = ";
    var_dump($it->current());
    $it->next();
  }
}

function main() {
  $v = Vector {1, 2, 3};
  foo(new FrozenVector($v));
}

main();
