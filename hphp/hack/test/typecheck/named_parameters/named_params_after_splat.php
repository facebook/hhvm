<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

// TODO: Features that need to be implemented for named parameters

function take_many(named int $n, int $x1, int $x2, int $x3): void {}

function test_splat_before_named(): void {
  $ints2 = tuple(1, 1);
  $ints3 = tuple(1, 1, 1);
  $ints4 = tuple(1, 1, 1, 1);

  // OK
  take_many(1, ...$ints2, n=1);
  // OK
  take_many(...$ints3, n=1);

  // Error: too many arguments
  take_many(1, ...$ints3, n=1);
  // Error: too many arguments
  take_many(...$ints4, n=1);
}
