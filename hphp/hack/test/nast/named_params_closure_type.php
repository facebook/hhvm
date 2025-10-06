<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function test((function(int, named int $x, optional named int $y): void) $f): void {}

function main(): void {
  test((int $_, named int $x, named int $y = 1) ==> {});
}

function bad((function(int $positional_param_with_name): void) $f): void {}
