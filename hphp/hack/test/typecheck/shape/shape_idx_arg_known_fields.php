<?hh // strict

function idxPassedInShapeWithKnownFields(shape('x' => int) $shape): void {
  Shapes::idx($shape, 'x');
  Shapes::idx($shape, 'y');
}
