<?hh

<<__Sealed(
  A::class,
  B::class // error, E is not a parent of B
)>>
enum E: int {}

enum A: int {
  use E;
}
enum B: int {}
