<?hh

interface I {
  abstract const X;
}

abstract class C implements I {
  public function getX() :mixed{
    return static::X;
  }
}

class D extends C {
  const X = 'D::C';
}


<<__EntryPoint>>
function main_abstract_const4() :mixed{
var_dump(D::X);
var_dump(C::X);
}
