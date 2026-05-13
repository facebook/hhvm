<?hh

function f(shape('x' => int, 'y' => int) $s): void {
  shape($x, $y) = $s;
}

function g(shape('a' => int, 'b' => int, 'c' => int) $s): void {
  shape($a, 'c' => $cc, $b) = $s;
}

function h(shape('long_variable_name_one' => int, 'long_variable_name_two' => int, 'long_variable_name_three' => int) $s): void {
  shape($long_variable_name_one, $long_variable_name_two, $long_variable_name_three) = $s;
}
