<?hh

function test($container): void {
  var_dump(array_key_exists(null, $container));
  var_dump(array_key_exists(42, $container));
  var_dump(array_key_exists('foo', $container));
}

<<__EntryPoint>>
function main(): void {
  test(vec[]);
  test(dict[]);
  test(vec[1, 2, 3]);
  test(dict['' => 42]);
}
