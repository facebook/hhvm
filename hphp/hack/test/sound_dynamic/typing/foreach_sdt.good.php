<?hh


class C {
  public function f() : void {}
}

<<__SupportDynamicType>>
function f(vec<C> $c) : void {
  $x = $c[0];
  $as = vec[$x]; // x is T on normal pass and dynamic on dyn pass
  foreach ($as as $a) {
    $a->f();
  }
}
