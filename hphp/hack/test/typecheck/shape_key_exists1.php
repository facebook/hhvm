<?hh // strict

// Correct usage of Shapes::keyExists

type s = shape('x' => int);

type t = shape('x' => ?int);

function test(bool $b, s $s, t $t): bool {
  if ($b) {
    $st = $s;
  } else {
    $st = $t;
  }
  Shapes::keyExists($s, 'x');
  Shapes::keyExists($t, 'x');
  return Shapes::keyExists($st, 'x');
}

function test2(bool $b): void {

  $s = shape('x' => 4);
  $t = shape('y' => 'aaa');

  if ($b) {
    $st = $s;
  } else {
    $st = $t;
  }
  Shapes::keyExists($s, 'x');
  Shapes::keyExists($t, 'y');
  Shapes::keyExists($st, 'x');
  Shapes::keyExists($st, 'y');
}
