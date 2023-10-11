<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function returnSomeOpenShapeType(): shape(?'foo' => string, ...) {
  return shape();
}

function mySandboxFunction(): mixed {
  if (true) {
    $s = returnSomeOpenShapeType();
  } else {
    $s = shape('bar' => 0);
  }
  return Shapes::idx($s, 'bar');
}
