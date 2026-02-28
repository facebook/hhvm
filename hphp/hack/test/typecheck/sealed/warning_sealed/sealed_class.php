<?hh

<<__Sealed(
  A::class,
  B::class // error, S is not a parent of B
)>>
class S {}

class A extends S {}
class B {}
