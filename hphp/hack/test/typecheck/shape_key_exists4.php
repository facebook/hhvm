<?hh // strict

// Nonexisting field in intersection of declared shapes

type s = shape(
  'x' => ?int,
  ...
);

type t = shape(
  'y' => ?string,
  ...
);

function test(bool $b, s $s, t $t): void {
  if ($b) {
    $st = $s;
  } else {
    $st = $t;
  }
  Shapes::keyExists($st, 'x');
}
