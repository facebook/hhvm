<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

final class VariadicNamedCallerBad {
  public function named_variadic(named int...): void {}
  public function positional_and_named_variadics(int ...$args, named string...): void {}
  public function fixed_and_named_variadic(named int $x, named string $y = 'default', named string...): void {}
}

function test_variadic_named_caller_bad(): void {
  $obj = new VariadicNamedCallerBad();
  $named_variadic = HH\meth_caller(VariadicNamedCallerBad::class, 'named_variadic');
  $named_variadic($obj, x='wrong type'); // Error
  $named_variadic($obj, 1); // Error

  $positional_and_named_variadics = HH\meth_caller(VariadicNamedCallerBad::class, 'positional_and_named_variadics');
  $positional_and_named_variadics($obj, 'wrong positional', x=2); // Error

  $fixed_and_named_variadic = HH\meth_caller(VariadicNamedCallerBad::class, 'fixed_and_named_variadic');
  $fixed_and_named_variadic($obj, extra='missing required'); // Error
  $fixed_and_named_variadic($obj, x='wrong fixed type'); // Error
  $fixed_and_named_variadic($obj, x=1, y=2, extra=2); // Error
}
//
