<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// The first variadic `named` parameter collects every named argument that
// matches no named parameter, so a second one can never be reached. This is a
// parse error; both options that gate the feature are enabled in this
// directory, so the gating errors do not show up.
//
// Each case also draws `Argument already bound: `...`` from naming, because an
// unnamed variadic named parameter lowers to the sentinel name `...` and the
// duplicates collide. That is pre-existing and independent of the check here.

function two(named int..., named string...): void {}

// A positional variadic in front does not change that.
function two_after_positional(int ...$rest, named int..., named string...): void {}

// Three reports once, on the second.
function three(named int..., named string..., named bool...): void {}

abstract class C {
  abstract public function m(named bool..., named int...): void;
}

interface I {
  public function j(named mixed..., named int...): void;
}

function in_lambdas(): void {
  $_ = (named int..., named string...) ==> 1;
  $_ = function(named int..., named string...): void {};
}

// In a function type hint too. This case was already rejected by the lowerer;
// it is now reported by the same check as the definitions above.
function in_fn_type((function(named int..., named string...): void) $_): void {}

// One of each is fine.
function one(int $x, int ...$rest, named arraykey...): void {}
function one_fn_type((function(int, int..., named arraykey...): void) $_): void {}
