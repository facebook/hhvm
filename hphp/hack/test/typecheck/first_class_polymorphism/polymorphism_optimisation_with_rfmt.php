<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

class AAA {}
interface III {}

abstract class WithRfmt<+T as arraykey> {
  abstract const type TC  as arraykey;
  public function foo(this::TC $_) : void {}
}

function refIt(): void {
  $i = meth_caller(WithRfmt::class, 'foo');
  hh_expect<HH\FunctionRef<(readonly function<TC0 as arraykey>(WithRfmt<arraykey> with { type TC = TC0 }, TC0): void)>>($i);
}
