<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

final class VariadicNamedCaller {
  public function named_variadic(named int...): void {}
  public function positional_and_named_variadics(int ...$args, named string...): void {}
  public function fixed_and_named_variadic(named int $x, named string $y = 'default', named string...): void {}
}

function test_variadic_named_caller(): void {
  $obj = new VariadicNamedCaller();
  $named_variadic = HH\meth_caller(VariadicNamedCaller::class, 'named_variadic');
  $named_variadic($obj);
  $named_variadic($obj, x=1, y=2);

  $positional_and_named_variadics = HH\meth_caller(VariadicNamedCaller::class, 'positional_and_named_variadics');
  $positional_and_named_variadics($obj);
  $positional_and_named_variadics($obj, 1, 2, x='ok', y='ok');

  $fixed_and_named_variadic = HH\meth_caller(VariadicNamedCaller::class, 'fixed_and_named_variadic');
  $fixed_and_named_variadic($obj, x=1);
  $fixed_and_named_variadic($obj, x=1, y='ok', extra='ok');
}
