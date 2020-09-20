<?hh

function f(mixed $x): void {
  $x is shape('x' => vec<_>);
  $x is shape('x' => shape('y' => vec<_>));
  $x is shape('x' => dict<_, _>);
  $x is shape('x' => int, 'y' => dict<_, _>);
  $x is shape('x' => shape('y' => dict<_, _>));
  $x as shape('x' => vec<_>);
  $x as shape('x' => shape('y' => vec<_>));
  $x as shape('x' => dict<_, _>);
  $x as shape('x' => int, 'y' => dict<_, _>);
  $x as shape('x' => shape('y' => dict<_, _>));
}
