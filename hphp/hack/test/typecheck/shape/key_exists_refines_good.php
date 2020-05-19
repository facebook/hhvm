<?hh // strict

function expect_int(int $_): void {}

function expect_closed_shape(shape('x' => int) $_): void {}

function expect_open_shape_1(shape('x' => int, ...) $_): void {}

function expect_open_shape_2(shape('x' => int, 'y' => mixed, ...) $_): void {}

function test1(shape(?'x' => int) $s): void {
  if (Shapes::keyExists($s, 'x')) {
    expect_int($s['x']);
    expect_closed_shape($s);
  }
}

function test2(shape(?'x' => int, ...) $s): void {
  if (Shapes::keyExists($s, 'x')) {
    expect_int($s['x']);
    expect_open_shape_1($s);
  }
}

function test3(shape('x' => int, ...) $s): void {
  if (Shapes::keyExists($s, 'y')) {
    expect_open_shape_2($s);
  }
}
