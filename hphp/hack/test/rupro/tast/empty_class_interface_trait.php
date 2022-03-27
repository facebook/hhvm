<?hh

interface I {}
interface J extends I {}

trait T {}

class A {
  use T;
}
class B<T> extends A implements I {}
