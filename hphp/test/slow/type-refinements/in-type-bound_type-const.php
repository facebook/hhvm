<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type T;
}

class Covar<+T> {}

interface I {
  abstract const type TOkInAs as Box with { type T = int };
  abstract const type TOkInAsNested as Covar<Box with { type T = int }>;
}
