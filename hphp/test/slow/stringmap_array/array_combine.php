<?hh

function main() {
  $keys = msarray(
    'discarded1' => 'key1',
    'discarded2' => 'key2',
    'discarded3' => 'key3',
    'discarded4' => msarray(
      'a string' => 'a value',
    ),
  );
  $values = msarray(
    'a' => 'value1',
    'b' => 'value2',
    'c' => 'value3',
    'd' => msarray(
      'a' => 2,
    ),
  );
  $res = array_combine($keys, $values);
  $res['Array'][10] = 'warning';
  var_dump($res);
  var_dump(array_combine($keys, array()));
  var_dump(array_combine(array(), $values));
}

main();
