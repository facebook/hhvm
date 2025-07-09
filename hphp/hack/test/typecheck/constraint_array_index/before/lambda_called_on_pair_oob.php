<?hh
function f(): void {
  $f = ($v ==> $v[2]);
  $f(Pair {"a", "b"});
}
