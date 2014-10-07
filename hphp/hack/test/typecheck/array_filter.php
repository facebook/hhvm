<?hh

function test_array_filter(
  array $untyped_array,
  array<?string> $vector_array,
  array<int, ?string> $hashtable_array,
  Container<?string> $container,
  KeyedContainer<int, ?string> $keyed_container,
  int $int,
  $untyped,
): void {
  $f = $x ==> true;
  $intersection_type = $int ? $container : $keyed_container;

  hh_show(array_filter($untyped_array));
  hh_show(array_filter($vector_array));
  hh_show(array_filter($hashtable_array));
  hh_show(array_filter($container));
  hh_show(array_filter($keyed_container));
  hh_show(array_filter($int));
  hh_show(array_filter($untyped));
  hh_show(array_filter($intersection_type));

  hh_show(array_filter($untyped_array, $f));
  hh_show(array_filter($vector_array, $f));
  hh_show(array_filter($hashtable_array, $f));
  hh_show(array_filter($container, $f));
  hh_show(array_filter($keyed_container, $f));
  hh_show(array_filter($int, $f));
  hh_show(array_filter($untyped, $f));
  hh_show(array_filter($intersection_type, $f));
}
