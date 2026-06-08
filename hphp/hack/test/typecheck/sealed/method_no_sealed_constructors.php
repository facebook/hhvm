<?hh

class C {
  <<__Sealed(D::class)>>
  public function __construct() {}
}

class D{}
