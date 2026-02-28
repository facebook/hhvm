<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type Acatg = shape(?'a' => ?bool, ...);
function eirqoeiurht(): ?bool {
  $s = make_acatg();
  $t = make_acatg();
  $u = make_acatg();
  $b = true;
  if (
    Shapes::keyExists($s, 'a') ||
    Shapes::keyExists($t, 'a') ||
    Shapes::keyExists($u, 'a')
  ) {
    $b = Shapes::idx($s, 'a') ?? Shapes::idx($t, 'a') ?? Shapes::idx($u, 'a');
  }
  return $b;
}

function make_acatg(): Acatg {
  return shape('a' => true);
}
