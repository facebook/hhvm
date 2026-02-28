<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

class AAA {}
interface III {}

abstract class Contrav<-T super AAA super III> {
  public function foo(): void {}
  public function bar(): this{ throw new Exception(); }
}

function refIt(): void {
  $f = meth_caller(Contrav::class, 'foo');
  hh_expect<HH\FunctionRef<(readonly function(Contrav<(AAA | III)>): void)>>($f);
}
