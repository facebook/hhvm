<?hh

function redundant_as_false_positive_on_dynamic_pass(
  Map<string, mixed> $map
): void {
  $traversable = idx($map, 'traversable') as Traversable<_>;

  $vector = new Vector($traversable);

  foreach ($vector as $element) {
    $element as KeyedContainer<_, _>;
  }
}
