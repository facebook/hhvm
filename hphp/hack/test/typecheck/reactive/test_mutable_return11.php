<?hh // strict

interface A {
  <<__Rx, __MutableReturn>>
  public function f(A $a): A;
}

class B implements A {
  <<__Rx>>
  public function f(A $a): A {
    // not ok - mismatched __MutableReturn on interface and implementation
    return $a;
  }
}
