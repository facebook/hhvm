<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

class A {}

function f(
  A with {
    const type T = int
  } $_
): void {}
