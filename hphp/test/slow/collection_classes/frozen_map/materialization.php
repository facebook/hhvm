<?hh

// Test ImmMap's materialization methods.

function show_iter($iter) :mixed{
  $vs = new Vector($iter);
  usort(inout $vs,  HH\Lib\Legacy_FIXME\cmp<>);
  echo "...\n";
  foreach ($vs as $v) var_dump($v);
  echo "...\n";
}

function main() :mixed{
  $fm = new ImmMap(Map {"a" => 10, "b" => 30, 10 => 42});

  echo "\ntoDArray...\n";
  $res = $fm->toDArray();
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
  var_dump($res is Vector);
  show_iter($res);

  echo "\ntoImmVector...\n";
  $res = $fm->toImmVector();
  var_dump($res is ImmVector);
  show_iter($res);

  echo "\ntoMap...\n";
  $res = $fm->toMap();
  var_dump($res is Map);
  show_iter($res);

  echo "\ntoImmMap...\n";
  $res = $fm->toImmMap();
  var_dump($res is ImmMap);
  show_iter($res);

  echo "\ntoSet...\n";
  $res = $fm->toSet();
  var_dump($res is Set);
  show_iter($res);

  echo "\ntoImmSet...\n";
  $res = $fm->toImmSet();
  var_dump($res is ImmSet);
  show_iter($res);

  echo "\ntoMap...\n";
  $res = $fm->toMap();
  var_dump($res is Map &&
           $res == Map { "a" => 10, "b" => 30, 10 => 42 });

  // TODO: add toImmMap() once it exists.
}


<<__EntryPoint>>
function main_materialization() :mixed{
main();
}
