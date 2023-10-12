<?hh // strict

function idxPassedInShapeWithUnknownFields(
  shape(
    'x' => int,
    ...
  ) $shape,
): void {
  Shapes::idx($shape, 'x');
  Shapes::idx($shape, 'y');
}
