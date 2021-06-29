<?hh

abstract class A {
  abstract const type T = int;
}

class B extends A {
  // T has been concretized
  // const type T = int;
}

interface I {
  const type T = string;
}

class C extends B implements I {
  // concrete I::T conflicts with concrete B::T
}
