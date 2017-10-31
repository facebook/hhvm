<?hh // strict

function idxPassedInShapeWithUnknownFieldsRemove(
  shape('x' => int) $shape,
): void {
  Shapes::idx($shape, 'x');
  Shapes::removeKey($shape, 'y');
  // $shape has unknown fields with 'y' being explicitly unset
  // this should be okay; the following should return null:
  Shapes::idx($shape, 'y');
}
