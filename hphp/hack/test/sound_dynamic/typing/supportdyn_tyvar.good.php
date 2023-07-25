<?hh


function make<T>(supportdyn<T> $x):Vector<supportdyn<T>> {
  return Vector{$x};
}

<<__SupportDynamicType>>
class C {
  public function foo(): void {}
}
function testit(bool $b):void {
  $x = make(null);
  // Because we have an invariant type variable, we end up with supportdyn<#0>
  $y = $x[0];
  if ($b) $y = new C();
  $y?->foo();
}
