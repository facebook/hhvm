<?hh

function unsafe_cast_lower_bound3(shape('my_integer' => int, 'my_string' => string, 'my_bool' => bool) $m): void {
  \HH\FIXME\UNSAFE_CAST<mixed, int>($m); // Lint
}
