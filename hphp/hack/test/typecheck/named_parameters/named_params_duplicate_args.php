<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

// Tests for duplicate named arguments (should be an error)

function simple_func(int $a, named int $b, named string $c = "default"): void {}

function test_duplicate_named_args(): void {
  simple_func(1, b=2, c="test");

  // Error: Duplicate named argument 'b'
  simple_func(1, b=2, b=3, c="test");

  // Error: Duplicate named argument 'c'
  simple_func(1, b=2, c="first", c="second");

  // Error: Duplicates
  simple_func(1, b=2, c="first", b=3, c="second");
}
