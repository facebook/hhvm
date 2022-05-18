<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class A { abstract const ctx C; abstract const type T; }

type AWithTIntSub = A with { type T as int; };
type AWithSuperDefaults = A with { ctx C super [defaults]; };

class G<T> {}

type GA<Ta as A with {
  type T = int;
  ctx C as [globals] super [defaults];
}> = G<Ta>;

type ANoTrailingSemiSingleMember = A with { type T = int };
type ANoTrailingSemiMultiMember = A with {
  ctx C = [];
  type T as arraykey
};

type AWithEmptyRefinement = A with {};
