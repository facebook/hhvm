<?hh

function f(mixed $x): void {
  $x as shape('y' => _);
  $x as shape('y' => int, 'z' => _);
  $x as shape('y' => _, 'z' => string);
  $x as shape('y' => _, 'z' => _);
  $x as shape('y' => shape('z' => _));
  $x as shape('y' => _, 'w' => shape('z' => _));
}
