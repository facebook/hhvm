<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

class AAA {}
interface III {}

abstract class Cov<+T as AAA as III> {
  public function foo(): void {}
  public function bar(): this{ throw new Exception(); }
}

function refIt(): void {
  $g = meth_caller(Cov::class, 'foo');
  hh_expect<HH\FunctionRef<(readonly function(Cov<(AAA & III)>): void)>>($g);
}
