<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type T;
}

class C {
  const type Tb = Box with { type T = int };
}
