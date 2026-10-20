<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

//
// One of each is fine.
function one(int $x, int ...$rest, named arraykey...): void {}
function one_fn_type((function(int, int..., named arraykey...): void) $_): void {}
//
