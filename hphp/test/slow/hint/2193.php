<?hh

class C {
}
class D {
}
class X {
  public function f($x1, $x2 = null, $x3 = 123, string $x4,
                    string $x5 = null, string $x6 = "abc",
                    AnyArray $x7, AnyArray $x8 = null, C $x9,
                    D $x10 = null, bool $x11, bool $x12 = true,
                    int $x13, int $x14 = 73, float $x15,
                    float $x16 = 1.5, float $x17) :mixed{
}
}
function main() :mixed{
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

<<__EntryPoint>>
function main_2193() :mixed{
main();
}
