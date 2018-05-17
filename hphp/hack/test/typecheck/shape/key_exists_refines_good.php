<?hh // strict

function expect_int(int $_): void {}

function expect_closed_shape(shape('x' => int) $_): void {}

function expect_open_shape_1(shape('x' => int, ...) $_): void {}

function expect_open_shape_2(shape('x' => int, 'y' => mixed, ...) $_): void {}

function test1(shape('x' => int) $s): void {
  if (Shapes::keyExists($s, 'x')) { // always true
    expect_int(Shapes::idx($s, 'x'));
  }
}

function test2(shape(?'x' => int) $s): void {
  if (Shapes::keyExists($s, 'x')) {
    expect_int(Shapes::idx($s, 'x'));
    expect_closed_shape($s);
  }
}

function test3(shape('x' => int) $s): void {
  if (Shapes::keyExists($s, 'y')) { // always false
  }
}

function test4(shape('x' => int, ...) $s): void {
  if (Shapes::keyExists($s, 'x')) { // always true
    expect_int(Shapes::idx($s, 'x'));
  }
}

function test5(shape(?'x' => int, ...) $s): void {
  if (Shapes::keyExists($s, 'x')) {
    expect_int(Shapes::idx($s, 'x'));
    expect_open_shape_1($s);
  }
}

function test6(shape('x' => int, ...) $s): void {
  if (Shapes::keyExists($s, 'y')) {
    expect_open_shape_2($s);
  }
}

function test7(shape('x' => int, ...) $s): void {
  Shapes::removeKey(inout $s, 'x');
  if (Shapes::keyExists($s, 'x')) {
  }
}
