<?hh

newtype st = ?shape('z' => int);

function shape_key_exists(?shape('x' => int) $s1, shape('y' => int) $s2, ?st $s3): void {
  Shapes::idx($s1, 'x');
  Shapes::idx($s2, 'y');
  Shapes::idx($s3, 'z');
}
