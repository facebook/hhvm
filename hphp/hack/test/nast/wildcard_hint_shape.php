<?hh

function wilcard_hint_shape_toplevel(mixed $x): void {
     $x as shape('y' => _);
     $x as shape('y' => int, 'z' => _);
     $x as shape('y' => _, 'z' => string);
     $x as shape('y' => _, 'z' => _);
     $x as shape('y' => shape('z' => _));
     $x as shape('y' => _, 'w' => shape('z' => _));
}

function wildcard_hint_shape_nested(mixed $x): void {
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
