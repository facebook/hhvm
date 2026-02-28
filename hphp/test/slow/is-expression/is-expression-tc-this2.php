<?hh

class C {
  const type T = this;
}
class D /* no relationship to C */ {
  public function isCT(mixed $x): void {
    if ($x is C::T) {
      echo "C::T\n";
    } else {
      echo "not C::T\n";
    }
  }
  public function isET(mixed $x): void {
    if ($x is E::T) {
      echo "E::T\n";
    } else {
      echo "not E::T\n";
    }
  }
}
class E extends C {}


<<__EntryPoint>>
function main_is_expression_tc_this2() :mixed{
$c = new C();
$d = new D();
$e = new E();

$d->isCT($c);
$d->isCT($d);
$d->isCT($e);

echo "\n";

$d->isET($c);
$d->isET($d);
$d->isET($e);
}
