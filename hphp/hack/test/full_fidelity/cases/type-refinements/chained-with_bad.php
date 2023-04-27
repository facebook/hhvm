<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface I { const type T1; const type T2; }

type TBadAmbiguous = I with { type T1 = int } with { type T1 = string };

type TBadMinimal = I with {} with {};
