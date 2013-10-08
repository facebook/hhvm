<?hh

// Test the KeyedIterable interface.

function foo(KeyedIterable $fv) {
  echo "\nmap...\n";
  var_dump($fv->map(function ($v) { return 2 * $v; }));

  echo "\nmapWithKey...\n";
  var_dump($fv->mapWithKey(function ($k, $v) { return Pair {$k, $v}; }));

  echo "\nfilter...\n";
  var_dump($fv->filter(function ($v) { return $v <= 1; }));

  echo "\nfilterWithKey...\n";
  var_dump($fv->filterWithKey(function ($k, $v) { return $k + $v <= 3; }));

  echo "\nzip...\n";
  var_dump($fv->zip(Vector {4, 5}));

  echo "\nkvzip...\n";
  var_dump($fv->kvzip());

  echo "\nkeys...\n";
  var_dump($fv->keys());

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
