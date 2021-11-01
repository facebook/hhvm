<?hh

trait B {}
trait C {}

class A {
  use
    B,
    C,
    ;
}
