<?hh

interface I {
  abstract const X;
}

abstract class C1 implements I {
  abstract const X;
}

class C implements I {
  const X = 'C::X';
}

abstract class D extends C {
  abstract const X; // fatal!
}
