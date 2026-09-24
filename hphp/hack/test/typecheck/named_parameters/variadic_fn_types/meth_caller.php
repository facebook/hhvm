<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class MethCallerNamedVariadic {
  public function variadic(named int...): void {}

  public function fixed_and_variadic(named int $x, named int...): void {}

  public function mixed(int $x, named int...): void {}

  public function both_variadics(int ...$args, named int...): void {}
}

function test_meth_caller_named_variadic(): void {
  HH\meth_caller(MethCallerNamedVariadic::class, 'variadic');
  HH\meth_caller(MethCallerNamedVariadic::class, 'fixed_and_variadic');
  HH\meth_caller(MethCallerNamedVariadic::class, 'mixed');
  HH\meth_caller(MethCallerNamedVariadic::class, 'both_variadics');
}
