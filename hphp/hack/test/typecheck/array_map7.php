<?hh

function f(string $_): bool { return true; };
function g(string $_, string $__): bool { return true; };

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
  hh_show(array_map($f, $vector_array));
  hh_show(array_map($f, $hashtable_array));
  hh_show(array_map($f, $untyped));
  hh_show(array_map($f, $container));
  hh_show(array_map($f, $intersection));
  hh_show(array_map($f, $vector));
  hh_show(array_map($g, $intersection, $vector_array));
}
