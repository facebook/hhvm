<?hh

function asdfjfasdf(shape('a' => int) $s): void {
  if (Shapes::keyExists($s, 'a')) {
    $s['a'];
  } else {
    $s['a'];
  }
  expect_shape_a($s); // This should be ok
}

function asdfjfasdf2(shape(?'a' => int) $s): void {
  if (Shapes::keyExists($s, 'a')) {
    $s['a'];
  } else {
    $s['a'];
  }
  expect_shape_opta($s); // This should be ok
}

function expect_shape_a(shape('a' => int) $_): void {}
function expect_shape_opta(shape(?'a' => int) $_): void {}
