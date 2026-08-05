<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// With variadic_named_parameters=false, a variadic named parameter on a
// function definition is rejected at naming time.
function take_named_variadic(named arraykey...): void {}

function take_mixed(int $x, named string...): void {}

abstract class C {
  abstract public function m(named bool...): void;
  public static function s(int $a, named string...): void {}
}

interface I {
  public function j(named mixed...): void;
}

function in_lambdas(): void {
  $_ = (int $x, named arraykey...) ==> $x;
  $_ = function(named int...): void {};
  $_ = async (named int...) ==> 1;
}

// Regular variadic (positional) still works without the flag.
function take_regular(int ...$xs): void {}

// A named (non-variadic) parameter also still works without the flag.
function take_named(named int $x): void {}
