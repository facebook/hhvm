<?hh // strict

<<__Sealed(A::class)>>
trait T {}

abstract class A {
  use T;
}
