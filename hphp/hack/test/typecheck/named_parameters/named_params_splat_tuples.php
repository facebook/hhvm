<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

// Tests for splatting tuples with named parameters

function take(named int $n, int $x1, int $x2): void {}

function take_many(named int $n, int $x1, int $x2, int $x3): void {}

function test_splat_with_named(): void {
  $numbers = tuple(1, 2);
  $more_numbers = tuple(2, 3, 5);

  // OK: Named parameter followed by splat
  take(n=1, ...$numbers);

  // OK: Named parameter with regular args and splat
  take_many(n=1, ...$more_numbers);

  // OK: Named parameter with mixed regular and splat
  take_many(n=1, 2, 3, 5);

  // Error: Missing required named parameter
  take(...$numbers);

  // Error: Too many arguments with splat
  $too_many = tuple(1, 2, 3, 4, 5);
  take(n=1, ...$too_many);
}
