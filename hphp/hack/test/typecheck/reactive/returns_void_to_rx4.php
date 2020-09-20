<?hh // partial
class A {
  <<__Rx, __Mutable>>
  public function f(): A {
    return new A();
  }
}

class B extends A {
  // ERROR:
  <<__Rx, __Mutable, __ReturnsVoidToRx>>
  public function f(): A {
    return $this;
  }
}

<<__Rx, __MutableReturn>>
function f(): A {
  return new B();
}
<<__Rx>>
function g(): void {
  $a = \HH\Rx\mutable(f());
  $a1 = $a->f();
}
