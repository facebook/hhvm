<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// A variadic `named` parameter collects arguments nothing can refer to, so it
// cannot be given a name. This is a parse error; both options that gate the
// feature are enabled in this directory, so only the parse errors show up.

function with_name(int $x, named arraykey ...$rest): void {}

// Same tokens, different spacing.
function with_name_attached(int $x, named arraykey... $rest): void {}

abstract class C {
  abstract public function m(named bool ...$flags): void;
}

interface I {
  public function j(named mixed ...$rest): void;
}

function in_lambdas(): void {
  $_ = (named int ...$r) ==> 1;
  $_ = function(named int ...$r): void {};
}

// In a function type hint too.
function in_fn_type((function(named int $x...): void) $_): void {}

// The unnamed form is what this syntax looks like; no parse error here.
function without_name(int $x, named arraykey...): void {}
function without_name_fn_type((function(named int...): void) $_): void {}
