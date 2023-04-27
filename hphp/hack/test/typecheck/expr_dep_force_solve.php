<?hh

function apply<T1, T2>((function(T1, T2): T1) $f, T1 $v1, T2 $v2): T1 {
  return $f($v1, $v2);
}

interface C {
  public function foo(C $other): C;
}

function test(C $value): void {
  // Here we don't want the call $a->foo(...) to force
  // solve T1 to (<expr#1> as C), or else the call will
  // not typecheck.
  apply(($a, $b) ==> $a->foo($b), $value, $value);
}
