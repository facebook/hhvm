<?hh

function f(?shape('a' => int) $s): void {
  Shapes::idx($s, 'a'); // Fine
  Shapes::idx($s, 'b'); // Not fine
  $s as nonnull;
  Shapes::idx($s, 'a'); // Fine
  Shapes::idx($s, 'b'); // Not fine
}
