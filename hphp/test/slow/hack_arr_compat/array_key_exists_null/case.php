<?hh

function test_array_key_exists($key, $container): ?bool {
  try {
    var_dump(array_key_exists($key, $container));
  } catch (InvalidArgumentException $e) {
    echo $e->getMessage().PHP_EOL;
  }
}

function test($container): void {
  test_array_key_exists(null, $container);
  test_array_key_exists(42, $container);
  test_array_key_exists('foo', $container);
}

<<__EntryPoint>>
function main(): void {
  test(vec[]);
  test(dict[]);
  test(keyset[]);
  test(vec[1, 2, 3]);
  test(dict['' => 42]);
  test(keyset[1, 2, 3]);
}
