<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

final class NamedCallerBad {
  public function required(int $positional, named int $x, named string $y): string {
    return $y;
  }

  public function optional(named int $x = 0): int {
    return $x;
  }

  public function positional_variadic(int ...$args, named string $tag): string {
    return $tag;
  }
}

function expect_int(int $_): void {}

function test_named_caller_bad(): void {
  $obj = new NamedCallerBad();
  $required = HH\meth_caller(NamedCallerBad::class, 'required');
  $required($obj, 1, y='missing x'); // Error
  $required($obj, 1, x='wrong type', y=3); // Error
  $required($obj, 1, x=2, y='ok', unknown=4, x=3); // Error
  $required($obj, 1, 2, 'positional not named'); // Error
  $required($obj, x=2, y='missing positional'); // Error
  $required(new stdClass(), 1, x=2, y='wrong receiver'); // Error
  expect_int($required($obj, 1, x=2, y='wrong return')); // Error

  $optional = HH\meth_caller(NamedCallerBad::class, 'optional');
  $optional($obj, x='wrong type', unknown=3); // Error

  $positional_variadic = HH\meth_caller(NamedCallerBad::class, 'positional_variadic');
  $positional_variadic($obj, 1, 2); // Error
  $positional_variadic($obj, 1, 'wrong positional', tag=3); // Error
}
//
