<?hh

function main() {
  $key = 'miarray_key';
  $val = miarray(
    1 => 'foo',
    2 => 'bar',
  );
  apc_store($key, $val);

  $val_read = apc_fetch($key);
  $val_read['ffff'] = 'warning';
  var_dump($val_read);
}

main();
