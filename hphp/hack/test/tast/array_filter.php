<?hh // partial

function test_array_filter(
  array $untyped_array,
  array<?string> $vector_array,
  Container<?string> $container,
  KeyedContainer<int, ?string> $keyed_container,
  $untyped,
  bool $b,
): void {
  $unresolved = $b ? $container : $keyed_container;

  array_filter($untyped_array);
  array_filter($vector_array);
  array_filter($container);
  array_filter($keyed_container);
  array_filter($untyped);
  array_filter($unresolved);

  $f = (?string $x): bool ==> true;

  array_filter($untyped_array, $f);
  array_filter($vector_array, $f);
  array_filter($container, $f);
  array_filter($keyed_container, $f);
  array_filter($untyped, $f);
  array_filter($unresolved, $f);
}
