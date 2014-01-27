<?php

class C {
}
class D {
}
class X {
  public function f_mixed($x1, $x2 = null, $x3 = 123) { }
  public function f_string(string $x4, string $x5 = null, string $x6 = "abc") { }
  public function f_array(array $x7, array $x8 = null) { }
  public function f_class(C $x9, D $x10 = null) { }
  public function f_bool(bool $x11, boolean $x12 = true) { }
  public function f_int(int $x13, integer $x14 = 73) { }
  public function f_double(real $x15, double $x16 = 1.5, float $x17 = -1.5) { }
}
function main() {
  $rc = new ReflectionClass('X');
  $clsDecl = "class Y extends X {\n";
  foreach ($rc->getMethods() as $rf) {
    $params = $rf->getParameters();
    $first = true;
    $clsDecl .= '  public function '.$rf->getName().'(';
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
    $clsDecl .= ") {}\n";
  }
  $clsDecl .= "}\n";
  echo $clsDecl;
}
main();
