<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type T;
  abstract const ctx C;
}

function refined_ctx_as_type(Box with { type C = mixed } $_): void {}

function refined_type_as_ctx(Box with { ctx T = [] } $_): void {}
