<?hh // strict

function test(): void {
  $s = shape('x' => 4, 'y' => 'aaa');
  $s['x']; // no error
  Shapes::removeKey(inout $s, 'x'); // no error for safe-pass-by-ref
  $s['y']; // no error
  $s['x']; // error
}
