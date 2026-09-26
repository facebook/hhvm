<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function takes_named_variadic_generic<T>(T $x, named string...): void {}

function test(): void {
  takes_named_variadic_generic(other = 's', 1);
//                                      ^ enforcement-at-caret
}
