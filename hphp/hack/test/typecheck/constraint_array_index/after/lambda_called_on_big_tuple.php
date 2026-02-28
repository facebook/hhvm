<?hh
function f(): void {
  $f = ($v ==> $v[1]);
  $f(tuple("a", 0, tuple()));
}
