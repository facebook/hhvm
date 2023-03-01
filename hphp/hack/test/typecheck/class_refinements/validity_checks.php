<?hh

<<file: __EnableUnstableFeatures('type_refinements')>>

class C {}

abstract class D extends C {
  abstract const type T as arraykey;
}

class E extends D {
  const type T = string;
}

interface Bad {
  abstract const type BAD0 as C with { type T = int };
  abstract const type BAD1 as D with { ctx T = [] };
}

interface Good {
  abstract const type OK0 as D with { type T = int };
  abstract const type OK1 as E with { type T = string };
}
