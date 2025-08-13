<?hh
<<file:__EnableUnstableFeatures('named_parameters_use')>>

// TODO(named_parameters): make $x a named parameter
// when we add syntax for named params in declarations
function foo(int $x): void {}

function main(): void {
  // TODO(named_parameters): support this
  foo(x = 3);
  $x = vec[1,2,3];
  foo(...$x, x = 3);
}
