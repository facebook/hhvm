<?hh

interface I {
  abstract const X;
}

abstract class C implements I {
  public function getX() {
    return static::X;
  }
}

class D extends C {
  const X = 'D::C';
}

var_dump(D::X);
var_dump(C::X);
