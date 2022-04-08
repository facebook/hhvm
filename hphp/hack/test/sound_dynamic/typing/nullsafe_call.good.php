<?hh

<<__SupportDynamicType>>
class C {
  public function f() : int { return 0; }
}

function test(?C $c) : void {
  $c?->f();
}
