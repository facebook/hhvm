<?hh

class C {}

class D extends C {}

<<__SupportDynamicType>>
class V<T> {
  public function filter(~supportdyn<(function(T): bool)> $fn): ~V<T> { return $this; }
}

function f(~V<C> $v) : void {
  $v->filter($x ==> $x is C || $x is D);
}
