<?hh
function f(): void {
  $f = ($v ==> $v[1]);
  $f(Pair {"a", "b"});
}
