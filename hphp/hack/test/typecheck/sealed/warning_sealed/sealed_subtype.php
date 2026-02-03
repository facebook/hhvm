<?hh

<<__Sealed(
  A::class,
  B::class // error, I is not a parent of B
)>>
interface I {}

class A implements I {}
class B {}
