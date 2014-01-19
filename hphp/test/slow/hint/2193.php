<?php

class C {
}
class D {
}
class X {
  public function f($x1, $x2 = null, $x3 = 123, string $x4,
                    string $x5 = null, string $x6 = "abc",
                    array $x7, array $x8 = null, C $x9,
                    D $x10 = null, bool $x11, boolean $x12 = true,
                    int $x13, integer $x14 = 73, real $x15,
                    double $x16 = 1.5, float $x17) {
}
}
function main() {
  $rc = new ReflectionClass('X');
  $rf = $rc->getMethod('f');
  $params = $rf->getParameters();
  $first = true;

  $clsDecl = "class Y extends X {
public function f(";
  foreach ($params as $rp) {
    if (!$first) $clsDecl .= ', ';
    $first = false;
    $th = $rp->getTypehintText();
    if ($th) {
      $clsDecl .= ($th . ' ');
    }
    $clsDecl .= ('$' . $rp->getName());
    if ($rp->isDefaultValueAvailable()) {
      $clsDecl .= (' = ' . $rp->getDefaultValueText());
    }
  }
  $clsDecl .= ") {}
}
";
  echo $clsDecl;
}
main();
