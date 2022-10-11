<?hh
<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type T;
}

function as_arg(classname<Box with { type T = int }> $cls): void {}
