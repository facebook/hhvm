<?hh

function f(shape('x' => int, ?'y' => int) $s): void {
  shape('x' => $x, ?'y' => $y) = $s;
}

function g(shape('long_required_field' => int, ?'long_optional_field' => int, ?'another_long_optional' => string) $s): void {
  shape('long_required_field' => $long_required_field, ?'long_optional_field' => $long_optional_field, ?'another_long_optional' => $another_long_optional) = $s;
}
