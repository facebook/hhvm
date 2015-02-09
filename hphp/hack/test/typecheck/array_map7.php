<?hh

function f(string $_): bool {
  return true;
}

function g(string $_, string $__): bool {
  return true;
}

function test(
  array $array,
  array<string> $vector_array,
  array<X, string> $hashtable_array,
  $untyped,
  KeyedContainer<X, string> $keyed_container,
  Container<string> $container,
  Vector<string> $vector,
) {
  $f = fun('f');
  $g = fun('g');
  $intersection = true ? $vector_array : $keyed_container;

  hh_show(array_map($f, $array));
  take_bool_array(array_map($f, $vector_array));
  take_X_bool_array(array_map($f, $hashtable_array));
  hh_show(array_map($f, $untyped));
  take_arraykey_bool_array(array_map($f, $container));
  take_mixed_bool_array(array_map($f, $intersection));
  take_bool_array(array_map($f, $vector));
  take_bool_array(array_map($g, $intersection, $vector_array));
}

function take_bool_array(array<bool> $_): void {}
function take_X_bool_array(array<X, bool> $_): void {}
function take_arraykey_bool_array(array<arraykey, bool> $_): void {}
function take_mixed_bool_array(array<mixed, bool> $_): void {}
