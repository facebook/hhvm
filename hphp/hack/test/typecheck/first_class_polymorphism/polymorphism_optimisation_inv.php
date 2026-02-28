<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

class AAA implements III {}
interface III {}

abstract class Inv<T super AAA as III> {
  public function foo(): void {}
  public function bar(): this{ throw new Exception(); }
}

function refIt(): void {
  $h = meth_caller(Inv::class, 'foo');
  hh_expect<HH\FunctionRef<(readonly function<T super AAA as III>(Inv<T>): void)>>($h);
}
