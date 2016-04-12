<?hh

newtype myshape = shape(
  'x' => int,
  'y' => string,
);

// Nested shapes don't grow
function test(): myshape {
  $v = Vector {shape('x' => 4)};
  $v[0]['y'] = 'aaa';
  return $v[0];
}
