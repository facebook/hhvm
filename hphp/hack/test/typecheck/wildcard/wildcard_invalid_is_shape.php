<?hh

function f(mixed $x): void {
  $x is shape('y' => _);
  $x is shape('y' => int, 'z' => _);
  $x is shape('y' => _, 'z' => string);
  $x is shape('y' => _, 'z' => _);
  $x is shape('y' => shape('z' => _));
  $x is shape('y' => _, 'w' => shape('z' => _));
}
