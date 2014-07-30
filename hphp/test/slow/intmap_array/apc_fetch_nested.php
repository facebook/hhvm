<?hh

function main() {
  $key = 'nested_miarray_key';
  $val = Map {
    1 => miarray(
      1 => Vector { 1, 2, 3, },
      2 => Map {
        1 => 'foo',
        2 => 'bar',
      },
      3 => Set {
        'foo', 'bar', 'baz',
      },
    ),
    2 => miarray(
      3 => miarray(
        4 => miarray(
          5 => Map {},
        ),
      ),
    ),
  };
  apc_store($key, $val);
  $val_read = apc_fetch($key);
  var_dump($val_read);
  var_dump($val);
  var_dump($val == $val_read);
}

main();
