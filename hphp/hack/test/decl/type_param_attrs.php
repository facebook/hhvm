<?hh

class A implements HH\TypeParameterAttribute {
  public function __construct() {}
}

class C<<<__Enforceable, A>> reify T1> {}
