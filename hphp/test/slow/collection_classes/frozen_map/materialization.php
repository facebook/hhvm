<?hh

// Test ImmMap's materialization methods.

function show_iter($iter) {
  $vs = new Vector($iter);
  sort($vs);
  echo "...\n";
  foreach ($vs as $v) var_dump($v);
  echo "...\n";
}

function main() {
  $fm = new ImmMap(Map {"a" => 10, "b" => 30, 10 => 42});

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

  echo "\ntoImmVector...\n";
  $res = $fm->toImmVector();
  var_dump($res instanceof ImmVector);
  show_iter($res);

  echo "\ntoMap...\n";
  $res = $fm->toMap();
  var_dump($res instanceof Map);
  show_iter($res);

  echo "\ntoImmMap...\n";
  $res = $fm->toImmMap();
  var_dump($res instanceof ImmMap);
  show_iter($res);

  echo "\ntoSet...\n";
  $res = $fm->toSet();
  var_dump($res instanceof Set);
  show_iter($res);

  echo "\ntoImmSet...\n";
  $res = $fm->toImmSet();
  var_dump($res instanceof ImmSet);
  show_iter($res);

  echo "\ntoMap...\n";
  $res = $fm->toMap();
  var_dump($res instanceof Map &&
           $res == Map { "a" => 10, "b" => 30, 10 => 42 });

  // TODO: add toImmMap() once it exists.
}

main();
