<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

// Tests for optional named parameters

function with_optional_params(
  int $a,
  named int $b = 42,
  named int $c = 43,
): void {}

function test_optional_params(): void {
  // OK: All parameters provided
  with_optional_params(1, b=2, c=3);

  // OK: Only providing required parameters
  with_optional_params(1);

  // OK: Only providing some optional parameters
  with_optional_params(1, b=2);
  with_optional_params(c=3, 1);

  // Error: Wrong type
  with_optional_params(1, b="");

  // Error: Providing unexpected named parameter
  with_optional_params(1, d=4);
}
