<?hh

// Test FixedMap's materialization methods.

function show_iter($iter) {
  $vs = new Vector($iter);
  sort($vs);
  echo "...\n";
  foreach ($vs as $v) var_dump($v);
  echo "...\n";
}

function main() {
  $fm = new FixedMap(Map {"a" => 10, "b" => 30, 10 => 42});

  echo "\ntoArray...\n";
  $res = $fm->toArray();
  var_dump(is_array($res));
  show_iter($res);

  echo "\ntoKeysArray...\n";
  $res = $fm->toKeysArray();
  var_dump(is_array($res));
  show_iter($res);

  echo "\ntoValuesArray...\n";
  $res = $fm->toValuesArray();
  var_dump(is_array($res));
  show_iter($res);

  echo "\ntoVector...\n";
  $res = $fm->toVector();
  var_dump($res instanceof Vector);
  show_iter($res);

  echo "\ntoFixedVector...\n";
  $res = $fm->toFixedVector();
  var_dump($res instanceof FixedVector);
  show_iter($res);

  echo "\ntoSet...\n";
  $res = $fm->toSet();
  var_dump($res instanceof Set);
  show_iter($res);

  echo "\ntoFixedSet...\n";
  $res = $fm->toFixedSet();
  var_dump($res instanceof FixedSet);
  show_iter($res);

  echo "\ntoMap...\n";
  $res = $fm->toMap();
  var_dump($res instanceof Map && $res == Map { "a" => 10, "b" => 30, 10 => 42 });

  // TODO: add toFixedMap() once it exists.
}

main();
