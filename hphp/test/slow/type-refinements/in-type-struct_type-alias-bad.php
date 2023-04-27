<?hh
<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type T;
}

type AliasBad = Box with { type T = int };
