<?hh

interface I {}
interface J extends I {}

trait T {}

class A {
  use T;
}
class B extends A implements I {}
