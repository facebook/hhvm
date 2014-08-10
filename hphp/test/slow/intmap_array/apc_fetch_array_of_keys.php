<?hh

function main() {
  $val_one = miarray(
    1 => 'one',
    2 => 'two',
  );
  $val_two = miarray(
    3 => 'three',
    4 => 'four',
  );
  $values = msarray(
    'key_one' => $val_one,
    'key_two' => $val_two,
  );
  $keys = array_keys($values);
  apc_store($values);
  $vals_read = apc_fetch($keys);
  var_dump($vals_read);

  echo "Deleting keys...\n";
  apc_delete($keys);
  var_dump(apc_fetch($keys));

  echo "Modifying read values...\n";
  $val_one_read = $vals_read[$keys[0]];
  $val_one_read['possible warning'] = 'no warning since apc';
  var_dump($val_one_read);
  $val_one['possible warning'] = 'warning since original value';
  var_dump($val_one);
}

main();
