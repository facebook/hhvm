<?hh // strict

// Nonexisting field in intersection of ad-hoc shapes

function test2(bool $b): void {

  $s = shape('x' => 4);
  $t = shape('y' => 'aaa');

  if ($b) {
    $st = $s;
  } else {
    $st = $t;
  }
  Shapes::keyExists($st, 'z');
}
