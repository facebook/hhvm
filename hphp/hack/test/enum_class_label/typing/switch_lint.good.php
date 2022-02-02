<?hh

interface I {}
class C implements I {}
class D implements I {}

enum class E : I {
  C A = new C();
  D B = new D();
  D X = new D();
}

function foo(HH\EnumClass\Label<E, I>  $label) : void {
  switch ($label) {
    case E#A : break;
    case E#B : break;
    default: break;
  }
}

function bar<T as HH\EnumClass\Label<E, I>>(T $label) : T {
  switch ($label) {
    case E#A : break;
    case E#B : break;
    default: break;
  }
  return $label;
}
