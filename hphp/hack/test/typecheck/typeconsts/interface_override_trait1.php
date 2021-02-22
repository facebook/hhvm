<?hh

interface I1 {
  const type T = int;
}

trait T1 implements I1 {}

interface I {
  const type T = int;
}

class A implements I { use T1; }
