<?hh //strict

// Removing field from shape

function test(): void {
  $s = shape('x' => 4, 'y' => 'aaa');
  $s['x']; // no error
  Shapes::removeKey(inout $s, 'x');
  $s['y']; // no error
  $s['x']; // error
}
