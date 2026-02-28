<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function apply<T, Tr>((function(T): Tr) $f, T $x): Tr {
  return $f($x);
}

function revapply<T, Tr>(T $x, (function(T): Tr) $f): Tr {
  return $f($x);
}

class C {
  public function foo(): void {}
}
function test(): void {
  apply($x ==> $x->fo(), new C());
  revapply(new C(), $x ==> $x->fo());
}
