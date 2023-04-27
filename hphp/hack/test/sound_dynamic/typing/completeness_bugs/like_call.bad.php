<?hh

<<__SupportDynamicType>>
class C { public function p() : bool { return true; } }

<<__SupportDynamicType>>
class D<-T> {
  public function __construct() {}
}

<<__SupportDynamicType>>
function my_plambda<Tv as supportdyn<mixed>>(
  ~(function(Tv):bool) $lambda,
): ~D<Tv> {
  return new D();
}

function m(~(function(~D<C>):void) $f): void {
  $f(my_plambda($x ==> $x->p()));
}
