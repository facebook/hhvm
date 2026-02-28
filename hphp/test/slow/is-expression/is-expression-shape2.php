<?hh

function is_shape(mixed $x): void {
  if ($x is shape(
    'a' => int,
    'b' => bool,
    ?'c' => string,
  )) {
    echo "shape\n";
  } else {
    echo "not shape\n";
  }
}


<<__EntryPoint>>
function main_is_expression_shape2() :mixed{
is_shape(null);
is_shape(new stdClass());
is_shape(tuple(1, true));
is_shape(shape());
is_shape(shape('a' => 1));
echo "\n";
is_shape(shape('a' => 1, 'b' => true));
is_shape(shape('a' => '1', 'b' => true));
is_shape(shape('a' => 1, 'b' => 1.5));
is_shape(shape('a' => 1, 'b' => true, 'c' => 'c'));
is_shape(shape('a' => 1, 'b' => true, 'c' => null));
echo "\n";
is_shape(shape('a' => 1, 'b' => true, 'c' => 'c', 'd' => null));
is_shape(shape('a' => 1, 'b' => true, 'd' => null));
}
