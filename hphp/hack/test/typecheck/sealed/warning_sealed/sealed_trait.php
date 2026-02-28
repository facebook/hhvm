<?hh

<<__Sealed(
  A::class,
  B::class // error, T is not a parent of B
)>>
trait T {}

class A {
  use T;
}
class B {}
