<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

class C {
  <<__Sealed(D::class)>>
  public function __construct() {}
}

class D{}
