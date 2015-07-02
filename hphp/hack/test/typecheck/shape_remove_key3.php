<?hh //strict

// Reason for shape type should point to the last operation that modified it

function test(): string {
  $s = shape('x' => 4, 'y' => 'aaa');
  Shapes::removeKey($s, 'x');
  return $s;
}
