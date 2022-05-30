<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

type A = A with { type T as int; type T = int; };
