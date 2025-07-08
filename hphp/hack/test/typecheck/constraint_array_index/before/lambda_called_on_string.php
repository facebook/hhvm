<?hh
function f(): void {
  $f = ($v ==> $v[0]);
  $f("abc");
}
