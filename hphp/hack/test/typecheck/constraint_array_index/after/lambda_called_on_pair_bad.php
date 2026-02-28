<?hh
function f(): void {
  $f = ($v ==> $v["a"]);
  $f(Pair {"a", "b"});
}
