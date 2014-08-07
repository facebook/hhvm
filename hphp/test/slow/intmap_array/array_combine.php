<?hh

function main() {
  $keys = miarray(
    5 => 'key1',
    10 => 'key2',
    15 => 'key3',
    20 => miarray(
      1 => 2,
    ),
  );
  $values = miarray(
    -1 => 'value1',
    -2 => 'value2',
    -3 => 'value3',
    -4 => miarray(
      1 => 2,
    ),
  );
  $res = array_combine($keys, $values);
  $res['Array']['warning'] = true;
  var_dump($res);
  var_dump(array_combine($keys, array()));
  var_dump(array_combine(array(), $values));
}

main();
