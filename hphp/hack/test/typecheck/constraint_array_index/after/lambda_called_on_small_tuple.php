<?hh
function f(): void {
  $f = ($v ==> $v[1]);
  $f(tuple("a"));
}
