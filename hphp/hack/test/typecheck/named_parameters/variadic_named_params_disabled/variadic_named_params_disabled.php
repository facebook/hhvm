<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// With variadic_named_parameters=false, a variadic named parameter on a
// function definition is rejected at naming time.
function take_named_variadic(named int ...$xs): void {}

function take_mixed(int $x, named string ...$rest): void {}

// A method with a variadic named parameter is also rejected.
class C {
  public function m(named bool ...$flags): void {}
}

// Regular variadic (positional) still works without the flag.
function take_regular(int ...$xs): void {}

// A named (non-variadic) parameter also still works without the flag.
function take_named(named int $x): void {}
