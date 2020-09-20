<?hh
trait T1 {}

trait T2 {}

trait T3 {}

class C {
  use T1, T2, T3;

}
// trait using multiple traits
trait T {
  use T2, T3;
}


// class using different traits that use other traits
class D  {
  use T, T3;
}

class E extends C  {}

// class using trait which was used in indirect ancestor C
class F extends E {
  use T3;
}

interface I {}
interface I2 {}
trait Timplements implements I {

}

// I2 shouldn't be in the linearization of A
class A implements I2 {
  use Timplements;
}
