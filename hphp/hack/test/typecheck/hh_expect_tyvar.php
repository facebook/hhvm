<?hh

function f() : void {
  $v = Vector {};
  hh_expect<Vector<int>>($v);
  $v = Vector {};
  hh_expect_equivalent<Vector<int>>($v);
}
