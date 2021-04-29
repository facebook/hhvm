<?hh

namespace HH\Contexts {
  class C1 extends C2 {}
  class C2 extends C3 {}
  class C3 {}
  namespace Unsafe {
    type C1 = mixed; type C2 = mixed; type C3 = mixed;
  }

  abstract class A {
    abstract const ctx C super [C1] as [C3];
    public function set(this::C $x): void {}
  }
  class B extends A {
    const ctx C = [C2]; // equiv to: super C2 as C2
  }
  class D extends A {
    const ctx C = [C1];  // equiv to: super C2 as C2
  }

  function demo(A $a, B $b, D $d, C1 $c1, C2 $c2, C3 $c3): void {
    $a->set($c1);  // ok: C1 <: lower(A::C)=C1
    // $a->set($c2);  // err: C1 </: lower(A::C)=C1

    $b->set($c1); // ok: C1 <: lower(D::C)=C2
    $b->set($c2); // ok: C2 <: lower(D::C)=C2

    $d->set($c1); // ok: C1 <: lower(D::C)=C1
    // $d->set($c2); // err: C2 </: lower(D::C)=C1
  }

  abstract class A23 extends A {  // C is in [C1, C3]
    abstract const ctx C super [C2] as [C3]; // ok: C2 >: lower(A::C)
  }
  abstract class A2 extends A23 {
    abstract const ctx C super [C2] as [C2]; // ok: C2 <: upper(A2::C)
    // should behave similar to (see test_exactness* below):
    // const ctx C = [C2];
  }

  class Box<T> {}
  abstract class A2WithBoxC2 extends A2 {
    const type Td = Box<this::C>; // == Box<C2>
  }
  function test_exactness1(A2WithBoxC2::Td $x): Box<C2> {
    return $x;
  }
  function test_exactness2(Box<C2> $x): A2WithBoxC2::Td {
    return $x;
  }
}
