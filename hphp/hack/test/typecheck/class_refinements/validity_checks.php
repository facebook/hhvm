<?hh

<<file: __EnableUnstableFeatures('type_refinements')>>

class C {
}

abstract class D extends C {
  abstract const type T as arraykey;
}

class E extends D {
  const type T = string;
}

type BAD0 = C with { type T = int };
type BAD1 = D with { type T = C };
type BAD2 = E with { type T = int };

type OK0 = D with { type T = int };
type OK1 = E with { type T = string };
