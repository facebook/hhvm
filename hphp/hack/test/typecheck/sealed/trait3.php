<?hh

<<__Sealed(A::class)>>
trait T {}

abstract class A {
  use T; // ok
}

abstract class B {
  use T; // error
}

final class C extends A {} // ok
