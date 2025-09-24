<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function take(
  int $required,
  named string $label,
  int ...$more
): void {}

function test_varargs_with_named(): void {
  take(100, label="test", 1, 2, 3);
  take(100, label="test");
  take(label="test", 1, 2, 3);

  // Error: Missing required named parameter
  take(2, 3, 4);

  // Error: Missing required positional parameter
}
