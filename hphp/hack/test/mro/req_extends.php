<?hh
interface I {}
class C {}
trait T {
  require extends C;
  require implements I;
}
trait T2 {}

class D extends C implements I {
  use T;
}
