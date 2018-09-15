<?hh // strict

<<__Sealed(T2::class)>>
trait T {}

trait T2 {
  use T; // ok
}

trait T3 {
  use T; // error
}

trait T4 {
  use T2; // ok
}

abstract class A {
  use T2; // ok
}
