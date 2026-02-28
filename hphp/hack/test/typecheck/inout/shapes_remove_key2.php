<?hh

function test(): void {
  $s = shape('x' => 4, 'y' => 'aaa');
  Shapes::removeKey($s, 'x'); // error for safe-pass-by-ref
  $s['y']; // no error
}
