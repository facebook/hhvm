<?hh

<<__Sealed(
  A::class,
  2, // this is a parse error
  B::class // error, I is not a parent of B
)>>
interface I {}

class A implements I {}
class B {}
