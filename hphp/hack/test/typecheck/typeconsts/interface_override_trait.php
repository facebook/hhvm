<?hh

trait T1 {
  const type T = int;
}

interface I {
  const type T = int;
}

class A implements I { use T1; }
