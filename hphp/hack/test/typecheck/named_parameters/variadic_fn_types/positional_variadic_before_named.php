<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function positional_variadic_before_named(int ...$xs, named string $s): void {}

function test(): void {
  positional_variadic_before_named(1, 2, s = "ok");
  positional_variadic_before_named(s = "ok");
}
